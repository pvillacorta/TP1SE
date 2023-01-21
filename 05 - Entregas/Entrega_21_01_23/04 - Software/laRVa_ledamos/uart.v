
//---------------------------------------------
//		Core de UART
//	- Doble función de transmisor y receptor
//---------------------------------------------

module UART_CORE(
  output txd,    // Salida TX
  output tend,	 // Flag TX completa
  output thre,   // Flag Buffer TX vacío
  input [15:0]d, // Datos TX,BRG
  input wrtx,    // Escritura en TX
  input wrbaud,	 // Escritura en BRG
  output [7:0]q, // Datos RX
  output dv,     // Flag dato RX válido
  output fe,     // Flag Framing Error
  output ove,    // Flag Overrun
  input rxd,     // Entrada RX
  input rd,      // Lectura RX (borra DV)
  input clk
);

parameter BAUDBITS=9;

//////// BAUD Rate Generation /////////
reg [BAUDBITS-1:0]divider=0;
always @(posedge clk) if (wrbaud) divider<=d[BAUDBITS-1:0];

///////////////////////////////////////
// Transmisor
reg [7:0]thr;		// Buffer TX
reg thre=1;			// Estado THR 1: vacío, 0: con dato
reg [8:0]shtx=9'h1FF;	// Reg. desplazamiento de 9 bits 
reg [3:0]cntbit;	// Contador de bits transmitidos
reg rdy=1;			// Estado reg. despl. (1==idle)

// Divisor de TX 
reg [BAUDBITS-1:0] divtx=0;
wire clko;			// pulsos de 1 ciclo de salida
assign clko = (divtx==0);
always @ (posedge clk) 
    divtx <= (wrtx&rdy) ? 0 : (clko ? divider: divtx-1);

always @(posedge clk)
begin
    if (wrtx) begin      // Escritura en buffer THR
`ifdef SIMULATION
        $write ("%c",d&255); $fflush ( );
`endif
        thr<=d[7:0];
        thre<=1'b0;
    end
    
    if(rdy&(~thre)) begin      // Carga de reg. desp
        rdy<=1'b0;
        thre<=1'b1;
        shtx<={thr[7:0],1'b0}; // Incluido bit de START
        cntbit<=4'b0000;
    end
    if (clko) begin 
        if(~rdy) begin             // Desplazamiento de bits
            shtx<={1'b1,shtx[8:1]};
            cntbit<=cntbit+1;
            if (cntbit[3]&cntbit[0]) rdy<=1'b1; // 9 bits: terminado
        end
    end
end

assign txd = shtx[0];
assign tend = thre&rdy;

////////////////////////////////////////
// Receptor

/// Sincronismo de reloj
reg [1:0]rrxd=0; // RXD registrada dos veces
wire resinc;         // activa si cambio en RXD (resincroniza divisor)
wire falling;          // activa si flanco de bajada en RXD (para start)
always @(posedge clk) rrxd<={rrxd[0],rxd};
assign resinc = rrxd[0]^rrxd[1];
assign falling = (~rrxd[0])&rrxd[1];

/// Divisor
// Genera un pulso en mitad de la cuenta (centro de bit)
// se reinicia con resinc
reg [BAUDBITS-1:0] divrx=0;
wire shift;		// Pulso de 1 ciclo de salida
wire clki0;		// recarga de contador
assign shift = (divrx=={1'b0,divider[BAUDBITS-1:1]});
assign clki0= (divrx==0);

always @ (posedge clk) divrx <= (resinc|clki0) ? divider: divrx-1;

reg dv=0;               // Dato válido si 1
reg ove=0;              // Overrun
reg [8:0]shrx;          // Reg. desplazamiento entrada (9 bits para stop)
reg [7:0]rbr;           // Buffer RX
reg stopb;              // Bit de stop recibido
reg [3:0]cbrx=4'b1111;  // Contador de bits / estado (1111== idle)
wire rxst;              // Guardar shr en buffer si flanco subida
assign rxst=(cbrx==4'b1111);
reg rxst0;              // Para detectar flanco

always @(posedge clk)
begin
    rxst0<=rxst;
    if (rxst & falling) cbrx<=4'h9;  // START: 9 bits a recibir
    if (shift & (~rxst)) begin       // Desplazando y contando bits
        shrx<= #1 {rrxd[0],shrx[8:1]};
        cbrx<=cbrx-1;
    end
    if (rxst & (~rxst0)) begin   // Final de cuenta
        {stopb,rbr}<=shrx;       // Guardando dato y bit STOP
        dv<=1;                   // Dato válido
        ove<=dv;                 // Overrun si ya hay dato válido
    end

    if (rd) begin   // Lectura: Borra flags
        dv<=0;
        ove<=0;
    end
end

assign fe=~stopb;   // el Flag FE es el bit de STOP invertido
assign q = rbr;
endmodule

