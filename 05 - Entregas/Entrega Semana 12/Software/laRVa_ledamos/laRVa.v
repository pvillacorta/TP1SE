///////////////////////////////////////////////////////////////////////////////
//                laRVa RV32E / RV32I pipelined core                         //
//                       Jesús Arias (2022)                                  //
//                                                                           //
// Public Domain code (bugs included). Credits to:                           //
//  Claire Wolf (PicoRV core). The first I got working                       //
//  Bruno Levy & Matthias Koch (FemtoRV core). Much code was taken from here.//
///////////////////////////////////////////////////////////////////////////////

module laRVa (
		input  clk,			// 
		input  reset,		// active high, asynchronous
		output [31:2]addr,	// Bits 0 and 1 missing (words aligned to 32 bits)
		output [31:0]wdata,	// Data output
		output [3:0]wstrb,	// Byte strobes for writes
		input  [31:0]rdata,	// Data input
		input  irq,			// Interrupt, active high
		input  [31:2]ivector,// IRQ vector 
		output trap			// Trap ipterrupt requested for ECALL...
	);

///////////////////////////////////////////////
//////////////// register bank ////////////////
///////////////////////////////////////////////

parameter LOG2NREG=4;			// N registers = 2^log2nreg
parameter MAXREG=(1<<LOG2NREG)-1;

reg [31:0] regs [1:MAXREG];

integer i;
initial begin
	for (i=1; i<=MAXREG; i=i+1) regs[i]=0;
end

wire [4:0]rd;	// Destination register address
wire [4:0]rs1;	// Source 1 register address
wire [4:0]rs2;	// Source 2 register address
wire regswr;	// Write enable

wire [31:0]regsD;	// Input data to register bank
wire [31:0]regsQ1;	// Output 1	(to ALU)
wire [31:0]regsQ2;	// Output 2 (to ALU)
assign regsQ1=(rs1[LOG2NREG-1:0]!=0) ? regs[rs1[LOG2NREG-1:0]] : 32'h0;
assign regsQ2=(rs2[LOG2NREG-1:0]!=0) ? regs[rs2[LOG2NREG-1:0]] : 32'h0;

always @(posedge clk) begin
	if ((rd[LOG2NREG-1:0]!=0)&regswr) regs[rd[LOG2NREG-1:0]]<=regsD;
end

//////////////////////////////////////////////////////
////////      Instr Reg. and some decoding    ////////
//////////////////////////////////////////////////////
wire opload, opstore, opjal, opjalr, opbranch, opimm, opreg;
wire oplui, opauipc, opsystem, jump;

// Valid op-code flag
//  During the execution cycle of Loads, Stores, and Jumps
//  an invalid op-code is loaded into the IR register, and
//  this flag has to be set to zero
reg opvalid=0;	
always @(posedge clk or posedge reset) begin
	if (reset) opvalid<=0;
	else opvalid<=~(opload | opstore | jump | irqstart | mret);
end

// Instruction register
reg [31:0]IR;
always @(posedge clk) IR<=rdata;

// Decoding of op-code fields
wire [6:0]opcode=IR[6:0];
wire [2:0]funct3=IR[14:12];
wire [6:0]funct7=IR[31:25];

assign rd  = opvalid ? IR[11:7]: 0;
assign rs1 = opvalid ? IR[19:15]: 0;
assign rs2 = opvalid ? IR[24:20]: 0;

// The ten instruction opcodes
assign opreg    = opvalid & (opcode==7'b0110011);	// Rd = Rs1 op Rs2
assign opimm    = opvalid & (opcode==7'b0010011);	// Rd = Rs1 op imm
assign opload   = opvalid & (opcode==7'b0000011); 
assign opstore  = opvalid & (opcode==7'b0100011);
assign opbranch = opvalid & (opcode==7'b1100011);	// Branch if condition
assign opjal    = opvalid & (opcode==7'b1101111);
assign opjalr   = opvalid & (opcode==7'b1100111);
assign oplui    = opvalid & (opcode==7'b0110111);
assign opauipc  = opvalid & (opcode==7'b0010111);
assign opsystem = opvalid & (opcode==7'b1110011);

// Jumps: JAL, JALR, and taken Branches
assign jump = opjal | opjalr | (opbranch & predicate);
// Write register file on all instructions except Store, Branches, and System
assign regswr = (opreg|opimm|opload| opjal | opjalr | oplui | opauipc | csrrw_mepc);
// System opcodes supported
wire mret  = opsystem & (funct3==3'b000) & IR[21];	//Return from interrupts (user or machine)	
wire ecall = opsystem & (funct3==3'b000) & (IR[21:20]==2'b00);
wire ebreak= opsystem & (funct3==3'b000) & (IR[21:20]==2'b01);
wire csrrw_mepc = opsystem  & (funct3==3'b001) & (IR[31:20]==12'h341);
/////////////////////////
// Inmediate values
/////////////////////////
// The five immediate formats
wire [31:0] Uimm = {    IR[31],   IR[30:12], {12{1'b0}}};			// Upper
wire [31:0] Iimm = {{21{IR[31]}}, IR[30:20]};						// Imm
wire [31:0] Simm = {{21{IR[31]}}, IR[30:25],IR[11:7]};				// Store
wire [31:0] Bimm = {{20{IR[31]}}, IR[7],IR[30:25],IR[11:8],1'b0};	// Branch
wire [31:0] Jimm = {{12{IR[31]}}, IR[19:12],IR[20],IR[30:21],1'b0};	// JAL

//////////////////////////////////////////////////////
////////      ALU & other data processing     ////////
//////////////////////////////////////////////////////

// First ALU source
wire [31:0] aluIn1 = oplui ? 0 : (opauipc ? {PCci,2'b00} : regsQ1);

// Second ALU source
wire [31:0] aluIn2 = 
	( (opreg|opbranch)      ? regsQ2 : 32'h0 ) |
	( (opimm|opload|opjalr) ? Iimm   : 32'h0 ) |
	( opstore               ? Simm   : 32'h0 ) |
	( (oplui|opauipc)       ? Uimm   : 32'h0 ) ;

// The ALU has to subtract on SUB, SLT, SLTU, SLTI, SLTIU, and branches
wire issub = (opreg & (funct7[5] | (funct3==2) | (funct3==3))) |
			 (opimm & ((funct3==2) | (funct3==3))) | opbranch;

// Use a single 33 bits subtract (32+carry) for SUB and all comparisons
wire [32:0] aluAdder = {issub, (issub ? ~aluIn2 : aluIn2)} + aluIn1 + issub;
wire LT  = (aluIn1[31] ^ aluIn2[31]) ? aluIn1[31] : aluAdder[32];
wire LTU = aluAdder[32];
wire EQ  = (aluAdder[31:0] == 0);

  /////////////////////////////////////////////////
  // Barrel shifter
  /////////////////////////////////////////////////

wire [31:0]sh_in;	// Input to right shift
wire [31:0]sh_out;	// output
wire left_sh;		// Shift to left
wire signed_sh;		// Shift right preserving negatives
assign left_sh = (opreg | opimm) & (funct3==3'h1);

// reverse the bit order at the input for left shifts
assign sh_in = left_sh ? 
	{aluIn1[ 0],aluIn1[ 1],aluIn1[ 2],aluIn1[ 3],aluIn1[ 4],aluIn1[ 5],aluIn1[ 6],aluIn1[ 7],
	 aluIn1[ 8],aluIn1[ 9],aluIn1[10],aluIn1[11],aluIn1[12],aluIn1[13],aluIn1[14],aluIn1[15],
	 aluIn1[16],aluIn1[17],aluIn1[18],aluIn1[19],aluIn1[20],aluIn1[21],aluIn1[22],aluIn1[23],
	 aluIn1[24],aluIn1[25],aluIn1[26],aluIn1[27],aluIn1[28],aluIn1[29],aluIn1[30],aluIn1[31]
	} : aluIn1;
// Arithmetic shift right (sign preserved)
assign signed_sh = (opreg | opimm) & (funct3==3'h5) & funct7[5];
// The five layers of the barrel shifter
wire [31:0]sh_t1;
wire [31:0]sh_t2;
wire [31:0]sh_t3;
wire [31:0]sh_t4;
wire [31:0]sh_t5;
assign sh_t1 = aluIn2[4] ? {{16{ signed_sh & sh_in[31]}},sh_in[31:16]} : sh_in;
assign sh_t2 = aluIn2[3] ? {{ 8{ signed_sh & sh_t1[31]}},sh_t1[31: 8]} : sh_t1;
assign sh_t3 = aluIn2[2] ? {{ 4{ signed_sh & sh_t2[31]}},sh_t2[31: 4]} : sh_t2;
assign sh_t4 = aluIn2[1] ? {{ 2{ signed_sh & sh_t3[31]}},sh_t3[31: 2]} : sh_t3;
assign sh_t5 = aluIn2[0] ? {{  { signed_sh & sh_t4[31]}},sh_t4[31: 1]} : sh_t4;

// reverse the bit order at the output for left shifts
assign sh_out = left_sh ? 
	{sh_t5[ 0],sh_t5[ 1],sh_t5[ 2],sh_t5[ 3],sh_t5[ 4],sh_t5[ 5],sh_t5[ 6],sh_t5[ 7],
	 sh_t5[ 8],sh_t5[ 9],sh_t5[10],sh_t5[11],sh_t5[12],sh_t5[13],sh_t5[14],sh_t5[15],
	 sh_t5[16],sh_t5[17],sh_t5[18],sh_t5[19],sh_t5[20],sh_t5[21],sh_t5[22],sh_t5[23],
	 sh_t5[24],sh_t5[25],sh_t5[26],sh_t5[27],sh_t5[28],sh_t5[29],sh_t5[30],sh_t5[31]
	} : sh_t5;

  /////////////////////////////////////////////////
  // ALU output select
  /////////////////////////////////////////////////

wire [31:0] aluOut =
     (((funct3==0)|oplui|opauipc) ? aluAdder[31:0]  : 32'b0) |
     ((funct3==2)                 ? {31'b0, LT}     : 32'b0) |
     ((funct3==3)                 ? {31'b0, LTU}    : 32'b0) |
     ((funct3==4)                 ? aluIn1 ^ aluIn2 : 32'b0) |
     ((funct3==6)                 ? aluIn1 | aluIn2 : 32'b0) |
     ((funct3==7)                 ? aluIn1 & aluIn2 : 32'b0) |
     (((funct3==1)|(funct3==5))   ? sh_out          : 32'b0) ;

  //////////////////////////////////////////
  // The predicate for conditional branches.
  //////////////////////////////////////////
wire [7:0]prlist = {
	~LTU,	// 7: BGEU
	LTU,	// 6: BLTU
	~LT,	// 5: BGE
	LT,		// 4: BLT
	2'b00,	// 3,2: not used
	~EQ,	// 1: BNE
	EQ		// 0: BEQ
};
wire predicate = prlist[funct3];

//////////////////////////////////////////
// LOAD/STORE
//////////////////////////////////////////
// Address come from the adder in the ALU (rs1 + Imm)
// All memory accesses are aligned on 32 bits boundary. For this
// reason, we need some circuitry that does unaligned halfword
// and byte load/store, based on:
// - funct3[1:0]:  00->byte 01->halfword 10->word
// - aluAdder[1:0]: indicates which byte/halfword is accessed

   wire mem_byteAccess     =  funct3[1:0] == 2'b00;
   wire mem_halfwordAccess =  funct3[1:0] == 2'b01;

// address output              
//assign addr=(opload|opstore) ? aluAdder[31:2] : PC[31:2];
// a little bit faster...
wire [31:0]ldstaddr= regsQ1 + (opload? Iimm : Simm);
assign addr=(opload|opstore) ? ldstaddr[31:2] : PC;

// LOAD, in addition to funct3[1:0], LOAD depends on:
// - funct3[2]: 0->do sign expansion   1->no sign expansion

   wire LOAD_sign =
	(!funct3[2]) & (mem_byteAccess ? LOAD_byte[7] : LOAD_halfword[15]);

   wire [31:0] LOAD_data =
         mem_byteAccess ? {{24{LOAD_sign}},     LOAD_byte} :
     mem_halfwordAccess ? {{16{LOAD_sign}}, LOAD_halfword} :
                          rdata ;

   wire [15:0] LOAD_halfword =
	       aluAdder[1] ? rdata[31:16] : rdata[15:0];

   wire  [7:0] LOAD_byte =
	       aluAdder[0] ? LOAD_halfword[15:8] : LOAD_halfword[7:0];

// STORE

assign wdata[ 7: 0] = regsQ2[7:0];
assign wdata[15: 8] = aluAdder[0] ? regsQ2[7:0]  : regsQ2[15: 8];
assign wdata[23:16] = aluAdder[1] ? regsQ2[7:0]  : regsQ2[23:16];
assign wdata[31:24] = aluAdder[0] ? regsQ2[7:0]  :
			     aluAdder[1] ? regsQ2[15:8] : regsQ2[31:24];

// The memory write mask:
assign wstrb  = opstore ? 
	      mem_byteAccess      ?
	            (aluAdder[1] ?
		          (aluAdder[0] ? 4'b1000 : 4'b0100) :
		          (aluAdder[0] ? 4'b0010 : 4'b0001)
                    ) :
	      mem_halfwordAccess ?
	            (aluAdder[1] ? 4'b1100 : 4'b0011) :
              4'b1111:
              4'b0000;

///////////////////////////////////////////////
// The value written back to the register file.
///////////////////////////////////////////////

assign regsD  =
	((opreg | opimm | opauipc | oplui) ? aluOut     : 32'b0) |
	(opload                            ? LOAD_data  : 32'b0) |
	((opjal | opjalr)                  ? {PC,2'b00} : 32'b0) |
	(csrrw_mepc                        ? {PCreg[0],2'b00} : 32'b0);

//////////////////////////////////////////////////////
///////////////////      PC     //////////////////////
//////////////////////////////////////////////////////
// The 2 LSB bits of PC are always 00, and aren't implemented
// PC points one instruction ahead of the one being executed (4 bytes)
// Two register stack: PC[0]: normal mode (user)
//                     PC[1]: interrupts (machine)
// Additional register PCci:  address of current instruction 

reg  [31:2]PCreg[0:1];		// The Two PCs
wire [31:2]PC=PCreg[mmode];	// Current mode PC

wire [31:2]PC0=PCreg[0];	// For debug in gtkwave
wire [31:2]PC1=PCreg[1];

wire [31:2]PCimm=			// Immediate value to add to PC	
	((opbranch & predicate) ? Bimm[31:2] : 30'h0) |
	(opjal                  ? Jimm[31:2] : 30'h0);

// Next PC logic
wire [31:2]PCadd1= PC+(irqstart | opload | opstore ? 0 : 1); // Incremented/Same PC
wire [31:2]PCadd2= PCci+PCimm;								 // Jump address
wire [31:2]PCnext= opjalr ? aluAdder[31:2] : (jump? PCadd2 : PCadd1);

always @(posedge clk or posedge reset) begin
	if (reset) begin PCreg[0]<=0; PCreg[1]<=0; end
	else begin
		if (!mmode) PCreg[0]<= PCnext;
		else if (csrrw_mepc) PCreg[0]<=regsQ1[31:2]; 
		PCreg[1]<=(mmode&(~mret))? PCnext : ivector;
	end
end

// The PC value last cycle = PC of current instruction
reg [31:2]PCci;	
always @(posedge clk) PCci<=PC;

///////////////////////////////////////////////
// Interrupts:
// 	IRQ sequencer logic taken from GUS16
//////////////////////////////////////////////

reg q0=0;				// First FF (samples IRQ)
reg mmode=0;			// Machine mode: 0=normal, 1=IRQ 
wire ireti=mret&(~irq);	// Return from interrupt if no more IRQs pending
wire trap=ecall|ebreak;	// ECALL or EBREAK executed

always @(posedge clk or posedge reset ) 
	begin
	if (reset) begin 
		q0<=1'b0;
		mmode<=1'b0;
	end
	else begin
		q0 <= (~ireti) & (q0 | irq);
		mmode <= (~ireti) & (q0|mmode|trap);
	end
end

assign irqstart = (~mmode) & (q0|trap) ; // Single cycle pulse


endmodule
