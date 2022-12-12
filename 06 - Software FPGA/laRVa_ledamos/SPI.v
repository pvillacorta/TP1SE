
///////////////////////////////////////////////////////////////////
//					CONTROLADOR SPI MAESTRO
//
//  - Sólo modo SPI 0
//  - Divisor de reloj entre 2 y 512 (siempre par)
//  - Tamaño del dato variable entre 1 y 32 bits
//
///////////////////////////////////////////////////////////////////

module SPI_master(
	input  	clk,			// Reloj maestro (max 170MHz)
	input 	miso,
	input   wr,				// Write pulse (un ciclo clk min)
	input   [31:0]din,		// solo se usan los bits LSBs si bits<32
	input 	[7:0]divider,	// Fsck = Fclk/(2*(divider+1))
	input	[5:0]bits,		// Nº de bits (max=32, min=1)
	output 	sck,
	output	mosi,
	output  busy,			// Flag de ocupado
	output  [31:0]dout		// Contiene el dato recibido al final (LSBs)
);

///////// divisor de reloj //////

reg [7:0]divcnt=0;
wire zerodiv;
assign zerodiv=(divcnt==0);
always @(posedge clk) begin
	if (zerodiv | wr) divcnt<=divider;
	else divcnt<=divcnt-1;
end

//// SCK ////

reg sck=0;
wire falling;
always @(posedge clk) if (wr) sck<=1'b0; else if (zerodiv&busy) sck<=~sck;

assign falling=zerodiv&sck;

///// Contador de Bits /////

reg [5:0]bitcnt=0;
wire busy;
assign busy=~(bitcnt==0);
always @(posedge clk) begin
	if (wr) bitcnt<=bits;
	else if (falling & busy) bitcnt<=bitcnt-1;
end

///// Shifter //////

reg sin=0;	// serial input sampled @ posedge SCK
always @(posedge sck) sin<=miso;	// único FF que no es síncrono 100%

reg [31:0]shreg;
always @(posedge clk) begin
	if (wr) shreg<=din;
	else if (falling) begin
		shreg[7:0]<={shreg[6:0],sin};
		shreg[8] <=(bits>8 )&shreg[7];
		shreg[9] <=(bits>9 )&shreg[8];
		shreg[10]<=(bits>10)&shreg[9];
		shreg[11]<=(bits>11)&shreg[10];
		shreg[12]<=(bits>12)&shreg[11];
		shreg[13]<=(bits>13)&shreg[12];
		shreg[14]<=(bits>14)&shreg[13];
		shreg[15]<=(bits>15)&shreg[14];
		shreg[16]<=(bits>16)&shreg[15];
		shreg[17]<=(bits>17)&shreg[16];
		shreg[18]<=(bits>18)&shreg[17];
		shreg[19]<=(bits>19)&shreg[18];
		shreg[20]<=(bits>20)&shreg[19];
		shreg[21]<=(bits>21)&shreg[20];
		shreg[22]<=(bits>22)&shreg[21];
		shreg[23]<=(bits>23)&shreg[22];
		shreg[24]<=(bits>24)&shreg[23];
		shreg[25]<=(bits>25)&shreg[24];
		shreg[26]<=(bits>26)&shreg[25];
		shreg[27]<=(bits>27)&shreg[26];
		shreg[28]<=(bits>28)&shreg[27];
		shreg[29]<=(bits>29)&shreg[28];
		shreg[30]<=(bits>30)&shreg[29];
		shreg[31]<=(bits>31)&shreg[30];
	end
end
assign mosi=shreg[bits-1];	// Lo más simple en multiplexores ;)
assign dout=shreg;

endmodule

