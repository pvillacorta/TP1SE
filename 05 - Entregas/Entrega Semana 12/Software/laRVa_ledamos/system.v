// =======================================================================
// RISC-V things
// by Jesús Arias
//--------------------------------------------------------------------
// -> EDITED:
// Proyecto Datalogger for IoT Curso 2022-2023
// Fecha: 05/12/2022 
// Autor: Pablo Villacorta, Rubén Serrano, Óscar Martín y Andrés Martín
// Asignatura: Taller de Proyectos I
// File: system.v Se instancian todos los módulos diseñados en la FPGA
// =======================================================================

/*
	Description:
	A LaRVA RISC-V system with 8KB of internal memory, and three UART
	
	Memory map:
	
	0x00000000 to 0x00001FFF	Internal RAM (with inital contents)
	0x00002000 to 0x1FFFFFFF	the same internal RAM repeated each 8KB
	0x20000000 to 0xDFFFFFFF       xxxx
	0xE0000000 to 0xE00000FF    IO registers
	0xE0000100 to 0xFFFFFFFF    the same IO registers repeated each 256B

	IO register map (all registers accessed as 32-bit words):
	
      address  |      WRITE        |      READ
    -----------|-------------------|---------------
    0xE0000000 | UART0 TX data     |  UART0 RX data
    0xE0000004 | UART0 Baud Divider|  UART0 flags
    0xE0000008 | UART1 TX data     |  UART1 RX data
    0xE000000C | UART1 Baud Divider|  UART1 flags
	0xE0000010 | UART2 TX data     |  UART2 RX data
    0xE0000014 | UART2 Baud Divider|  UART2 flags
	           |                   |
	0xE0000020 | SPI0 TX data      |  SPI0 RX data 
    0xE0000024 | SPI0 Control      |  SPI0 flags 
    0xE0000028 | SPI0 Slave Select |  xxxx 
	0xE0000030 | SPI1 TX data      |  SPI1 RX data 
    0xE0000034 | SPI1 Control      |  SPI1 flags 
    0xE0000038 | SPI1 Slave Select |  xxxx 
	           |                   |
	0xE0000040 | I2C data/control  |  I2C data/status 
    0xE0000044 | I2C divider       |
	           |                   |
    0xE0000060 | MAX_COUNT [TMCF]  |  TIMER [tcount]
			   |                   | 
    0xE0000080 | GPOUT        	   |  GPOUT 
    0xE0000084 | GPOUT        	   |  GPIN 
               |                   | 
    0xE00000C0 | Interrupt Enable  |  Interrupt enable 
    0xE00000E0 | IRQ vector 0 Trap | 
    0xE00000E4 | IRQ vector 1 RX   | 
    0xE00000E8 | IRQ vector 2 TX   | 
    0xE00000EC | IRQ vector 3 Timer| 
    0xE00000F0 | IRQ vector 4 RX1  | 
    0xE00000F4 | IRQ vector 5 TX1  | 
    0xE00000F8 | IRQ vector 6 RX2  | 
    0xE00000FC | IRQ vector 7 TX2  | 

    UART Baud Divider: Baud = Fcclk / (DIVIDER+1) , with DIVIDER >=7
    
    UART FLAGS:    bits 31-5  bit 4  bit 3 bit 2 bit 1 bit 0
                     xxxx      OVE    FE    TEND  THRE   DV
        DV:   Data Valid (RX complete if 1. Cleared reading data register)
        THRE: TX Holding register empty (ready to write to data register if 1)
        TEND: TX end (holding reg and shift reg both empty if 1)
        FE:   Frame Error (Stop bit received as 0 if FE=1)
        OVE:  Overrun Error (Character received when DV was still 1)
        (DV and THRE assert interrupt channels #4 and #5 when 1)

    ------ 
    SPI Control:   bits 31-14  bits 13-8  bits 7-0 
                      xxxx        DLEN     DIVIDER 
        DLEN:    Data lenght (8 to 32 bits) 
        DIVIDER: SCK frequency = Fclk / (2*(DIVIDER+1)) 
         
    SPI Flags:     bits 31-1  bit 0 
                      xxxx     BUSY 
        BUSY:  SPI exchanging data when 1  

    SPI Slave Select: bits 31-2  bit 1   bit 0 
                         xxxx     ss1     ss0 
        ss0 : Selects the SPI slave 0 when 0 (active low) [BME680_CS] 
        ss1 : Selects the SPI slave 1 when 0 (active low) [ADC_CS]
    ------ 
    I2C Data/Control: bit 10  bit 9  bit 8  bits 7-0 
                       STOP   START   ACK     DATA 
        STOP:  Send Stop sequence 
        START: Send Start sequence 
        ACK:   ACK bit. Must be 1 on writes and 0 on reads except last one 
        DATA:  Data to write (Must be 0xFF on reads) 
          - ACK and DATA are ignored if START or STOP are one 
          - Do not set START and STOP simultaneously 
          - Repeated START is not supported 
          - Writing to this register sets the BUSY flag until the start, 

            stop, or data, is sent         

    I2C Data/Status:          bit 9  bit 8  bits 7-0 
                               BUSY   ACK     DATA 
        BUSY:  Controller busy if 1.  
        ACK:   Received ACK bit (for writes) 
        DATA:  Received data (for reads) 
        
    I2C Divider: bits 6-0 
        SCL frequency = Fclk /(4*(DIVIDER+1)) 
    ------ 
    MAX_COUNT: Holds the maximum value of the timer counter. When the timer 
        reaches this value gets reset and request an interrupt if enabled. 
        Writes to MAX_COUNT also resets the timer and its interrupt flag. 
    TIMER: the current value of the timer (incremented each clock cycle) 
        Reads also clear the interrupt flag. 
    ------ 
    GPOUT: General purpose outputs. 
    GPIN: General purpose inputs. 
	
	// GPOUT[7]   -  GPOUT[6]  	  - GPOUT[5]     - GPOUT[4]  - GPOUT[3] - GPOUT[2] - GPOUT[1] - GPOUT[0]
	// DUST_CTRL  -  GAS_1V4_CTRL - GAS_5V_CTRL  - STEPUP_CE - ice_led4 - ice_led3 - ice_led2 - ice_led1
	
    ------ 
	Interrupt enable:  

        bit 0: Not used 
        bit 1: Enable UART0 RX interrupt if 1 
        bit 2: Enable UART0 TX interrupt if 1 
        bit 3: Enable TIMER    interrupt if 1 
        bit 4: Enable UART1 RX interrupt if 1 
        bit 5: Enable UART1 TX interrupt if 1 
        bit 6: Enable UART2 RX interrupt if 1 
        bit 7: Enable UART2 TX interrupt if 1 

    Interrupt Vectors: Hold the addresses of the corresponding interrupt 
    service routines.          
         
*/

`include "laRVa.v"
`include "uart.v"
`include "SPI.v"

module SYSTEM (
	input clk,		// Main clock input 25MHz
	input reset,	// Global reset (active high)

	input	rxd0,	// UART0
	output 	txd0,
	
	input	rxd1,	// UART 1
	output 	txd1,	// 	

	input	rxd2,	// UART 2
	output 	txd2,	// 		
	
	output ice_sck,		// ICE SPI
	output ice_mosi,
	input  ice_miso,
	output ice_ss0,
	output ice_ss1,
	
	output iceLoRA_sck,		// ICE LORA SPI
	output iceLoRA_mosi,
	input  iceLoRA_miso,
	output iceLoRA_ss,
	output iceLoRA_RST,
	
	output gpout0,	// GPOUT
	output gpout1,	
	output gpout2,
	output gpout3,
	output gpout4,
	output gpout5,
	output gpout6,
	output gpout7,
	
	input gpin0,	// GPIN
	input gpin1,	
	input gpin2,	
	input gpin3,	
	input gpin4,
	input gpin5,
	input gpin6,
	input gpin7,
	
	
	
	output fssb	// Flash CS

);

wire		cclk;	// CPU clock
assign	cclk=clk;

///////////////////////////////////////////////////////
////////////////////////// CPU ////////////////////////
///////////////////////////////////////////////////////

wire [31:0]	ca;		// CPU Address
wire [31:0]	cdo;	// CPU Data Output
wire [3:0]	mwe;	// Memory Write Enable (4 signals, one per byte lane)
wire irq;
wire [31:2]ivector;	// Where to jump on IRQ
wire trap;			// Trap irq (to IRQ vector generator)

laRVa cpu (
		.clk     (cclk ),
		.reset   (reset),
		.addr    (ca[31:2] ),
		.wdata   (cdo  ),
		.wstrb   (mwe  ),
		.rdata   (cdi  ),
		.irq     (irq  ),
		.ivector (ivector),
		.trap    (trap)
	);

 
///////////////////////////////////////////////////////
///// Memory mapping
wire iramcs;
wire iocs;
// Internal RAM selected in lower 512MB (0-0x1FFFFFFF)
assign iramcs = (ca[31:29]==3'b000);
// IO selected in last 512MB (0xE0000000-0xFFFFFFFF)
assign iocs   = (ca[31:29]==3'b111);

// Input bus mux
reg [31:0]cdi;	// Not a register
always@*
 casex ({iocs,iramcs})
        2'b01: cdi<=mdo; 
        2'b10: cdi<=iodo;
        default: cdi<=32'hxxxxxxxx;
 endcase

///////////////////////////////////////////////////////
//////////////////// internal memory //////////////////
///////////////////////////////////////////////////////
wire [31:0]	mdo;	// Output data
ram32	 ram0 ( .clk(~cclk), .re(iramcs), .wrlanes(iramcs?mwe:4'b0000),
			.addr(ca[12:2]), .data_read(mdo), .data_write(cdo) );

//////////////////////////////////////////////////
////////////////// Peripherals ///////////////////
//////////////////////////////////////////////////

// ---> CONFIGURACIÓN DE LOS CHIP SELECT [CS]
wire uartcs;	// UART					at offset 0x00
wire spics;		// SPI					at offset 0x20
wire i2ccs;		// I2C					at offset 0x40
wire tempcs;	// Timer 				at offset 0X60
wire gpcs;		// GENERAL PURPOSE 		at offset 0x80
wire iencs;		// INTERRUPT ENABLE 	at offset 0xC0
wire irqcs;		// IRQEN 				at offset 0xE0
				//		 ...
				// other at offset 0xE0

assign uartcs = iocs&(ca[7:5]==3'b000);
assign spics  = iocs&(ca[7:5]==3'b001);
assign i2ccs  = iocs&(ca[7:5]==3'b010);
assign tempcs = iocs&(ca[7:5]==3'b011);
assign gpcs   = iocs&(ca[7:5]==3'b100);

assign iencs  = iocs&(ca[7:5]==3'b110);	//Interrupt Enable Chip Select
assign irqcs  = iocs&(ca[7:5]==3'b111);

// Peripheral output bus mux
reg [31:0]iodo;	// Not a register
always@*
 casex (ca[7:2])
	6'b000000: iodo<={24'h0,uart0_do}; 
	6'b000001: iodo<={27'h0,ove0,fe0,tend0,thre0,dv0};
	6'b000010: iodo<={24'h0,uart1_do}; 
	6'b000011: iodo<={27'h0,ove1,fe1,tend1,thre1,dv1};
	6'b000100: iodo<={24'h0,uart2_do}; 
	6'b000101: iodo<={27'h0,ove2,fe2,tend2,thre2,dv2};
	
	6'b001000: iodo<=rx_spi; 	    // SPI_RX 
	6'b001001: iodo<={31'h0,busy};  // SPI FLAG (busy)
	6'b001100: iodo<=rx_spiLoRA; 	    // SPI_LoRA_RX 
	6'b001101: iodo<={31'h0,busyLoRA};  // SPI_LoRA FLAG (busy)
	
	6'b100000: iodo<={24'h0,GPOUT}; // GPOUT
	6'b100001: iodo<={24'h0,gpin7,gpin6,gpin5,gpin4,gpin3,gpin2,gpin1,gpin0};  // GPIN
	
	//6'b010000: iodo<={...}; // I2C...
	//6'b010001: iodo<={...}; // I2C...
	
	
	
	6'b011xxx: iodo<=tcount; // TIMER
	6'b111xxx: iodo<={30'h0,irqen};
	default: iodo<=32'hxxxxxxxx;
 endcase

 /////////////////////////////
// GPOUT 

// GPIN: Siempre vale el dato de entrada.
// GPOUT: Se carga desde memoria

reg [7:0] GPOUT=0;

wire gpoutcs;
assign gpoutcs = gpcs & (~ca[2]); 

always @(posedge cclk)
begin
	if(gpoutcs & mwe[0]) // Escritura en GPOUTCS
		GPOUT <= cdo[7:0];
end

assign gpout0 = GPOUT[0];
assign gpout1 = GPOUT[1];
assign gpout2 = GPOUT[2];
assign gpout3 = GPOUT[3];
assign gpout4 = GPOUT[4];
assign gpout5 = GPOUT[5];
assign gpout6 = GPOUT[6];
assign gpout7 = GPOUT[7];

 /////////////////////////////
// TIMER

reg [31:0] tcount = 0;	// TCNT: Timer Counter Register
reg [31:0] TMR = 180000;  	// Time Match Register [MAX_COUNT: Holds the maximum value of the timer counter] // 4 seg: TMR = 32'h05F5E100; 
reg TMF  = 0;	//Time Match Flag Bit
reg TMF0 = 0;	//Time Match Flag Bit 0
reg TMF1 = 0;	//Time Match Flag Bit 1

// Necesitamos 3 registros ya que tenemos que mantener la señal
// de interrupcion durante al menos 2 ciclos de reloj!!!
// La señal de interrupcion posedge_TMF salta cuando TMF & ~TMF1

// Ciclo: 			TMF				TMF0			TMF1
//					 0				 0				 0		(No Interr)
//	tcount==TMR		 1				 0				 0		(Interr)
//					 1				 1				 0		(Interr)
//					 1				 1				 1		(No interr)	

wire TMF_wire;
wire rd_timer;
wire wr_timer;

assign rd_timer = tempcs & (mwe==4'b0000);
assign wr_timer	= tempcs & (mwe==4'b1111);
assign posedge_TMF   = ((~TMF1) & TMF);

always @(posedge cclk)
begin
	if(wr_timer) //Escritura en TMR
	begin
		TMF <= 0; //Desactivo el Flag de fin de cuenta al leer de timer / escribir
		TMR <= cdo[31:0];
		tcount <= 0;	//Reset del contador
	end else 
	begin
		if(rd_timer) //Lectura en TMR
			TMF <= 0; //Desactivo el Flag de fin de cuenta al leer de timer / escribir
		
		tcount <= tcount+1; //Incremento del contador
		TMF0<=TMF; 	// Cargo el TMF en TMF0
		TMF1<=TMF0; // Cargo el TMF0 en TMF1 
		if (tcount==TMR)
		begin
			tcount <= 0;	//Reset del contador
			TMF <= 1;	//Ativo el Flag de fin de cuenta
		end
	end
end


/////////////////////////////
// UART0

wire tend0,thre0,dv0,fe0,ove0; // Flags
wire [7:0] uart0_do;	// RX output data
wire uwrtx0;			// UART TX write
wire urd0;			// UART RX read (for flag clearing)
wire uwrbaud0;		// UART BGR write
// Register mapping
// Offset 0: write: TX Holding reg
// Offset 0: read strobe: Clear DV, OVE (also reads RX data buffer)
// Offset 1: write: BAUD divider
assign uwrtx0   = uartcs & (~ca[4])& (~ca[3])& (~ca[2]) & mwe[0];
assign uwrbaud0 = uartcs & (~ca[4])& (~ca[3])& (ca[2])  & mwe[0] & mwe[1];
assign urd0     = uartcs & (~ca[4])& (~ca[3])& (~ca[2]) & (mwe==4'b0000); // Clear DV, OVE flgas

UART_CORE #(.BAUDBITS(12)) uart0 ( .clk(cclk), .txd(txd0), .rxd(rxd0), 
	.d(cdo[15:0]), .wrtx(uwrtx0), .wrbaud(uwrbaud0),. rd(urd0), .q(uart0_do),
	.dv(dv0), .fe(fe0), .ove(ove0), .tend(tend0), .thre(thre0) );


/////////////////////////////
// UART1

wire tend1,thre1,dv1,fe1,ove1; // Flags
wire [7:0] uart1_do;	// RX output data
wire uwrtx1;			// UART TX write
wire urd1;				// UART RX read (for flag clearing)
wire uwrbaud1;		// UART BGR write			
// Register mapping
// Offset 0: write: TX Holding reg
// Offset 0: read strobe: Clear DV, OVE (also reads RX data buffer)
// Offset 1: write: BAUD divider
assign uwrtx1   = uartcs & (~ca[4])& (ca[3])& (~ca[2]) & mwe[0];
assign uwrbaud1 = uartcs & (~ca[4])& (ca[3])& (ca[2])  & mwe[0] & mwe[1];
assign urd1     = uartcs & (~ca[4])& (ca[3])& (~ca[2]) & (mwe==4'b0000); // Clear DV, OVE flgas

UART_CORE #(.BAUDBITS(12)) uart1 ( .clk(cclk), .txd(txd1), .rxd(rxd1), 
	.d(cdo[15:0]), .wrtx(uwrtx1), .wrbaud(uwrbaud1),. rd(urd1), .q(uart1_do),
	.dv(dv1), .fe(fe1), .ove(ove1), .tend(tend1), .thre(thre1) );

/////////////////////////////
// UART2

wire tend2,thre2,dv2,fe2,ove2; // Flags
wire [7:0] uart2_do;	// RX output data
wire uwrtx2;			// UART TX write
wire urd2;				// UART RX read (for flag clearing)
wire uwrbaud2;		// UART BGR write			
// Register mapping
// Offset 0: write: TX Holding reg
// Offset 0: read strobe: Clear DV, OVE (also reads RX data buffer)
// Offset 1: write: BAUD divider
assign uwrtx2   = uartcs & (ca[4])& (~ca[3])& (~ca[2]) & mwe[0];
assign uwrbaud2 = uartcs & (ca[4])& (~ca[3])& (ca[2])  & mwe[0] & mwe[1];
assign urd2     = uartcs & (ca[4])& (~ca[3])& (~ca[2]) & (mwe==4'b0000); // Clear DV, OVE flgas

UART_CORE #(.BAUDBITS(12)) uart2 ( .clk(cclk), .txd(txd2), .rxd(rxd2), 
	.d(cdo[15:0]), .wrtx(uwrtx2), .wrbaud(uwrbaud2),. rd(urd2), .q(uart2_do),
	.dv(dv2), .fe(fe2), .ove(ove2), .tend(tend2), .thre(thre2) );
	
/////////////////////////////
// ICE SPI

reg [13:0] spi_ctrl=0; 	 // Registro que contiene los valores de dlen y divider (procedentes de cdo)
reg [1:0]  spi_ss=2'b11; // Registro Slave Select 

wire[5:0]	dlen_spi;
wire[7:0]	divider_spi;

wire busy;
wire spi_wr;		// Señal de activación de escritura tanto en el control como en el rx/tx
wire spi_wr_ctrl;	// Señal de activación de escritura en el registro spi_control desde cdo
wire spi_wr_ss; 	// Señal de activación de escritura en el registro spi_ss desde cdo
wire[31:0]  rx_spi;

assign spi_wr = spics & (~ca[4]) & (~ca[3]) & (~ca[2]) & (mwe == 4'b1111); 
assign spi_wr_ctrl = spics & (~ca[4]) & (~ca[3]) & (ca[2]) & (mwe == 4'b1111);
assign spi_wr_ss = spics & (~ca[4]) & (ca[3]) & (~ca[2]) & (mwe == 4'b1111);

assign dlen_spi = spi_ctrl[13:8];
assign divider_spi = spi_ctrl[7:0];

assign ice_ss0 = spi_ss[0];
assign ice_ss1 = spi_ss[1];

always @(posedge cclk)
begin
	if(spi_wr_ctrl) //Escritura en spi_ctrl
		spi_ctrl <= cdo[13:0];
	if(spi_wr_ss)  	//Escritura en spi_ss
		spi_ss <= cdo[1:0];
end


SPI_master spi(
				.clk(cclk), .miso(ice_miso), .wr(spi_wr),
				.din(cdo[31:0]),
				.divider(divider_spi),.bits(dlen_spi),
				.sck(ice_sck), .mosi(ice_mosi), .busy(busy),			
				.dout(rx_spi)
			);


/////////////////////////////
// LORA SPI
	
reg [13:0] spiLoRA_ctrl=0; 	 // Registro que contiene los valores de dlen y divider (procedentes de cdo)
reg spiLoRA_ss=1'b1; // Registro Slave Select 
reg LoRA_RST = 0; // LoRA_RST

wire[5:0]	dlen_spiLoRA;
wire[7:0]	divider_spiLoRA;

wire busyLoRA;
wire spiLoRA_wr;		// Señal de activación de escritura tanto en el control como en el rx/tx
wire spiLoRA_wr_ctrl;	// Señal de activación de escritura en el registro spi_control desde cdo
wire spiLoRA_wr_ss; 	// Señal de activación de escritura en el registro spiLoRA_ss desde cdo
wire[31:0]  rx_spiLoRA;

assign spiLoRA_wr = spics & (ca[4]) & (~ca[3]) & (~ca[2]) & (mwe == 4'b1111); 
assign spiLoRA_wr_ctrl = spics & (ca[4]) & (~ca[3]) & (ca[2]) & (mwe == 4'b1111);
assign spiLoRA_wr_ss = spics & (ca[4]) & (ca[3]) & (~ca[2]) & (mwe == 4'b1111);

assign dlen_spiLoRA = spiLoRA_ctrl[13:8];
assign divider_spiLoRA = spiLoRA_ctrl[7:0];

assign iceLoRA_ss = spiLoRA_ss;
assign iceLoRA_RST = LoRA_RST;

always @(posedge cclk)
begin
	if(spiLoRA_wr_ctrl) //Escritura en spiLoRA_ctrl
		spiLoRA_ctrl <= cdo[13:0];
	if(spiLoRA_wr_ss)  	//Escritura en spiLoRA_ss
		spiLoRA_ss <= cdo[0];
end


SPI_master spiLoRA(
				.clk(cclk), .miso(iceLoRA_miso), .wr(spiLoRA_wr),
				.din(cdo[31:0]),
				.divider(divider_spiLoRA),.bits(dlen_spiLoRA),
				.sck(iceLoRA_sck), .mosi(iceLoRA_mosi), .busy(busyLoRA),			
				.dout(rx_spiLoRA)
			);
			
			
//////////////////////////////////////////
//    Interrupt control  

// -> IRQ enable reg (Registro de 8 bits) [bit0 unused]:

// IRQEN[7] - IRQEN[6] - IRQEN[5] - IRQEN[4] - IRQEN[3] - IRQEN[2] - IRQEN[1] - IRQEN[0]
//	U2TX	-  U2RX    -  U1TX    -  U1RX    -  TIMER   -  U0TX	   -  U0RX    -  unused 

reg [7:0]irqen=0;
always @(posedge cclk or posedge reset) begin
	if (reset) irqen<=0; else
	if (iencs &mwe[0]) irqen<=cdo[7:0];
end

// -> IRQ vectors

// IRQVEC[7] - IRQVEC[6] - IRQVEC[5] - IRQVEC[4] - IRQVEC[3] - IRQVEC[2] - IRQVEC[1] - IRQVEC[0]
//	U2TX	 -  U2RX     -  U1TX     -  U1RX     -  TIMER    -  U0TX	 -  U0RX     -  trap 

reg [31:2]irqvect[0:7]; //Array of 8 irqvectors

always @(posedge cclk) if (irqcs & (mwe==4'b1111)) irqvect[ca[4:2]]<=cdo[31:2];

// Enabled IRQs
wire [6:0]irqpen={
	irqen[7]&thre2, 		//irqpen[6] UART2TX
	irqen[6]&dv2,			//irqpen[5] UART2RX
	irqen[5]&thre1, 		//irqpen[4] UART1TX
	irqen[4]&dv1,			//irqpen[3] UART1RX
	irqen[3]&posedge_TMF,	//irqpen[2] TIMER. Posedge Time Match Flag	
	irqen[2]&thre0,			//irqpen[1] UART0 TX
	irqen[1]&dv0			//irqpen[0] UART0 RX
	};	// pending IRQs

// Priority encoder
wire [2:0]vecn = trap      ? 3'b000 : (	// ECALL, EBREAK: highest priority
				 irqpen[0] ? 3'b001 : (	// UART0 RX
				 irqpen[1] ? 3'b010 : (	// UART0 TX
				 irqpen[2] ? 3'b011 : (	// TIMER
				 irqpen[3] ? 3'b100 : (	// UART1 RX
				 irqpen[4] ? 3'b101 : (	// UART1 TX
				 irqpen[5] ? 3'b110 : (	// UART2 RX
				 irqpen[6] ? 3'b111 : 	// UART2 TX
				 			 3'bxxx )))))));	
assign ivector = irqvect[vecn];
assign irq = (irqpen!=0)|trap;

endmodule	// System




//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
//-- 32-bit RAM Memory with independent byte-write lanes
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////

module ram32
 (	input	clk,
	input	re,
	input	[3:0]	wrlanes,
	input	[10:0]	addr,
	output	[31:0]	data_read,
	input	[31:0] 	data_write
 );

reg [31:0] ram_array [0:2047];
reg [31:0] data_out;
        
assign data_read = data_out;
        
always @(posedge clk) begin
	if (wrlanes[0]) ram_array[addr][ 7: 0] <= data_write[ 7: 0];
	if (wrlanes[1]) ram_array[addr][15: 8] <= data_write[15: 8];
	if (wrlanes[2]) ram_array[addr][23:16] <= data_write[23:16];
	if (wrlanes[3]) ram_array[addr][31:24] <= data_write[31:24];
end

always @(posedge clk) begin
	if (re) data_out <= ram_array[addr];
end

initial begin
`ifdef SIMULATION
	$readmemh("rom.hex", ram_array);
`else
	$readmemh("rand.hex", ram_array);
`endif
end

endmodule

