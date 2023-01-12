// =======================================================================
// Core de UART - Numbits 
//	- Doble función de transmisor y receptor
// Fecha: 16/11/2022 
// Autores: Rubén Serrano, Andrés Martín Pablo Villacorta y Óscar Martín
// Asignatura: DISEÑO DE CIRCUITOS CON DISPOSITIVOS LOGICOS PROGRAMABLES
// File: uartnumbits.v UART con longitud de dato variable y stopbits configurable (1 o 2)
// =======================================================================

`timescale 1ns/10ps //%* ----- changed_3 -----*% 
// Es necesario para que el retardo de la [línea 160] sea de 1ns en lugar de 1 segundo 

module UART_CORE_NUMBITS(
  output txd,    // Salida TX
  output tend,	 // Flag TX completa
  output thre,   // Flag Buffer TX vacío
  input [15:0]d, // Datos TX,BRG
  input wrtx,    // Escritura en TX
  input wrconfig,	 // Escritura en BRG %* ----- changed_3 -----*%
  output [7:0]q, // Datos RX
  output dv,     // Flag dato RX válido
  output fe,     // Flag Framing Error
  output ove,    // Flag Overrun
  input rxd,     // Entrada RX
  input rd,      // Lectura RX (borra DV)
  input clk
);

parameter BAUDBITS=9;

//////// BAUD Rate Generation , numbits and stopconf /////////
reg [BAUDBITS-1:0]divider=0;
reg [1:0]numbits =0; 	//  %* ----- changed_3 -----*%: Incluimos el registro que nos va servir para controlar el número de bits
reg stopconf = 0; 	//  %* ----- changed_3 -----*%: Incluimos el registro que nos va servir para controlar el número de bits

always @(posedge clk) if (wrconfig) //%* ----- changed_3 -----*% INICIO
begin
	divider<=d[BAUDBITS-1:0]; // el divider lo hemos dejado como estaba
	numbits<=d[BAUDBITS+1:BAUDBITS]; // se han aprovechado d[10:9] para numbits
	stopconf<=d[BAUDBITS+2]; // se ha aprovechado d[11] para stopconf
end							//%* ----- changed_3 -----*% FIN



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

wire a7, a6, a5; // %* ----- changed_3 -----*% Se añaden 2 wire para tener las salidas de los combinacionales mostrados en informe
// con el mismo nombre, que se encuentran entre el registro thr y el shifter	

assign clko = (divtx==0);
always @ (posedge clk) 
    divtx <= (wrtx&rdy) ? 0 : (clko ? divider: divtx-1);
	
assign a7 = thr[7]|(numbits[1]|numbits[0]); // %* ----- changed_3 -----*%
assign a6 = thr[6]|numbits[1];// %* ----- changed_3 -----*%
assign a5 = thr[5]|(numbits[1]&numbits[0]);// %* ----- changed_3 -----*%

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
        shtx<={a7,a6,a5,thr[4:0],1'b0}; // Se meten salidas de los combinacionales al shifter  %* ----- changed_3 -----*%
        cntbit<=4'b0000;
    end
    if (clko) begin 
        if(~rdy) begin             // Desplazamiento de bits
            shtx<={1'b1,shtx[8:1]};
            cntbit<=cntbit+1;
			// %* ----- changed_3 -----*% INICIO
			case ({stopconf,numbits}) // Esto es un MUX dentro del bloque bit_counter. Según el valor de numbits
			// y de stopconf para parar la cuenta dependiendo de estas variables
				3'b000: if (cntbit==4'b1001) rdy<=1'b1; // 8 datos + 1 stop + 1 start = 10
				3'b001: if (cntbit==4'b1000) rdy<=1'b1; // 7 datos + 1 stop + 1 start = 9
				3'b010: if (cntbit==4'b0111) rdy<=1'b1; // 6 datos + 1 stop + 1 start = 8
				3'b011: if (cntbit==4'b0110) rdy<=1'b1; // 5 datos + 1 stop + 1 start = 7 MÍNIMA
				3'b100: if (cntbit==4'b1010) rdy<=1'b1; // 8 datos + 2 stop + 1 start = 11 MÁXIMA
				3'b101: if (cntbit==4'b1001) rdy<=1'b1; // 7 datos + 2 stop + 1 start = 10
				3'b110: if (cntbit==4'b1000) rdy<=1'b1; // 6 datos + 2 stop + 1 start = 9
				3'b111: if (cntbit==4'b0111) rdy<=1'b1; // 5 datos + 2 stop + 1 start = 8
				default: rdy<=1'bx;
			endcase// %* ----- changed_3 -----*% FIN
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
reg [BAUDBITS-1:0] divrx=0; // Generador de Baudios
wire shift;		// Pulso de 1 ciclo de salida
wire clki0;		// recarga de contador
assign shift = (divrx=={1'b0,divider[BAUDBITS-1:1]}); // 
assign clki0= (divrx==0);

always @ (posedge clk) divrx <= (resinc|clki0) ? divider: divrx-1; // SYN ****************************************************

reg dv=0;               // Dato válido si 1
reg ove=0;              // Overrun
reg [9:0]shrx;          // Reg. desplazamiento entrada (10 bits máximo) %* ----- changed_3 -----*%
reg [7:0]rbr;           // Buffer RX 
reg stopb;			// Pueden llegar hasta dos bits de stop 0
reg [3:0]cbrx=4'b1111;  // Contador de bits / estado (1111== idle)
wire rxst;              // Guardar shr en buffer si flanco subida
assign rxst=(cbrx==4'b1111);
reg rxst0;              // Para detectar flanco

always @(posedge clk)
begin
    rxst0<=rxst;
    if (rxst & falling)
	begin  // START: numero de bit a recibir dependiendo de numbits y stopconf
			case ({stopconf,numbits}) // Esto es un MUX dentro del bloque bit_counter. Según el valor de numbits
			// y de stopconf para parar la cuenta dependiendo de estas variables
				3'b000: cbrx<=4'b1001; // 8 datos + 1 stop = 9
				3'b001: cbrx<=4'b1000; // 7 datos + 1 stop = 8
				3'b010: cbrx<=4'b0111; // 6 datos + 1 stop = 7
				3'b011: cbrx<=4'b0110; // 5 datos + 1 stop = 6 MÍNIMA
				3'b100: cbrx<=4'b1010; // 8 datos + 2 stop = 10 MÁXIMA
				3'b101: cbrx<=4'b1001; // 7 datos + 2 stop = 9
				3'b110: cbrx<=4'b1000; // 6 datos + 2 stop = 8
				3'b111: cbrx<=4'b0111; // 5 datos + 2 stop = 7
				default: cbrx<=4'bxxxx;
			endcase// %* ----- changed_3 -----*% FIN
	end 
    if (shift & (~rxst)) begin       // Desplazando y contando bits
        shrx<= #1 {rrxd[0],shrx[9:1]}; // Retardo de 1ns
		//shrx<= {rrxd[0],shrx[9:1]};
        cbrx<=cbrx-1;
    end
    if (rxst & (~rxst0)) begin   // Final de cuenta
		case(stopconf)// %* ----- changed_3 -----*% INICIO
			//1'b0: {stopb,rbr} <= {shrx[9],(shrx[8:1]>>(numbits+1))};// Guardando dato y bit STOP. A STOPB le llegan 2 bits, mientras que antes llegaba 1.
			1'b0: {stopb,rbr} <= {shrx[9],(shrx[8:1]>>(numbits))};// Guardando dato y bit STOP. A STOPB le llegan 2 bits, mientras que antes llegaba 1.
			1'b1: {stopb,rbr} <= {shrx[9]&shrx[8],(shrx[7:0] >> numbits)};
			default {stopb,rbr} <= 9'bxxxxxxxxx;
		endcase// %* ----- changed_3 -----*% FIN
        dv<=1;                   // Dato válido
        ove<=dv;                 // Overrun si ya hay dato válido
    end

    if (rd) begin   // Lectura: Borra flags
        dv<=0;
        ove<=0;
    end
end
assign fe=~stopb;   // el Flag FE es el bit de STOP invertido **************************
assign q = rbr;
endmodule

