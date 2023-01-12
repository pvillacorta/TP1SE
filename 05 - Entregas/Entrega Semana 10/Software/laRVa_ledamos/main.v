//--------------------------------------------------------------------
// RISC-V things
// by Jesús Arias
//--------------------------------------------------------------------
`include "system.v"
`include "pll.v"

// Top module. (signals assigned to actual pins in file "pines.pcf")
module main(
	input  CLKIN, 		// Input clock from crystal oscillator (16MHz)
	// SPI
	output SCK,
	output MOSI,
	input  MISO,	
	output FSS,	// Flash SS
	// UART	
	input RXD,
	output TXD,
	// UART2	
	input RXD2,
	output TXD2
);

//-- PLL: generates a 25MHz master clock from a 16MHz input
wire clk,pll_lock;

pll
  pll1(
	.clock_in(CLKIN),
	.clock_out(clk),
	.locked(pll_lock)
	);

//assign clk=CLKIN;
//assign pll_lock=1'b1;

// Game controller and SPI pin mappings
assign JY4=1'b1;	// Game controller power
wire [7:0]pinin;

//assign XBHE=1'b0;
//assign XBLE=1'b0;

// Instance of the system
SYSTEM sys1( .clk(clk), .reset(reset),
		.txd(TXD), .rxd(RXD),
		.sck(SCK), .mosi(MOSI), .miso(MISO), .fssb(FSS),
		.txd2(TXD2), .rxd2(RXD2)
);

// Automatic RESET pulse: Reset is held active for 255 cycles after PLL lock

reg [21:0]cnt=22'h3fffff;
wire reset=(cnt!=0);

always @(posedge clk) cnt<= reset ? cnt-1: cnt;


//////////////////////////////////////////////////////
// Bidirectional data bus of the external RAM
// Tristates have to be instantiated usign the SB_IO module (specific of the ICE40 FPGA)

wire [15:0]xdi;	// internal data input bus
wire [15:0]xdo;	// internal data output bus

//////////////////////////////////////////////////////
// SB_IOs
wire oe;
assign oe=XOE; // activate tristates on writes (XOE inactive == High)

SB_IO #( .PIN_TYPE(6'b 1010_01), .PULLUP(1'b 0) ) xd_instance0
(   .PACKAGE_PIN(   XD[0]),
    .OUTPUT_ENABLE( oe   ),
    .D_OUT_0(       xdo[0]),
    .D_IN_0(        xdi[0]) );
SB_IO #( .PIN_TYPE(6'b 1010_01), .PULLUP(1'b 0) ) xd_instance1
(   .PACKAGE_PIN(   XD[1]),
    .OUTPUT_ENABLE( oe   ),
    .D_OUT_0(       xdo[1]),
    .D_IN_0(        xdi[1]) );
SB_IO #( .PIN_TYPE(6'b 1010_01), .PULLUP(1'b 0) ) xd_instance2
(   .PACKAGE_PIN(   XD[2]),
    .OUTPUT_ENABLE( oe   ),
    .D_OUT_0(       xdo[2]),
    .D_IN_0(        xdi[2]) );
SB_IO #( .PIN_TYPE(6'b 1010_01), .PULLUP(1'b 0) ) xd_instance3
(   .PACKAGE_PIN(   XD[3]),
    .OUTPUT_ENABLE( oe   ),
    .D_OUT_0(       xdo[3]),
    .D_IN_0(        xdi[3]) );
SB_IO #( .PIN_TYPE(6'b 1010_01), .PULLUP(1'b 0) ) xd_instance4
(   .PACKAGE_PIN(   XD[4]),
    .OUTPUT_ENABLE( oe   ),
    .D_OUT_0(       xdo[4]),
    .D_IN_0(        xdi[4]) );
SB_IO #( .PIN_TYPE(6'b 1010_01), .PULLUP(1'b 0) ) xd_instance5
(   .PACKAGE_PIN(   XD[5]),
    .OUTPUT_ENABLE( oe   ),
    .D_OUT_0(       xdo[5]),
    .D_IN_0(        xdi[5]) );
SB_IO #( .PIN_TYPE(6'b 1010_01), .PULLUP(1'b 0) ) xd_instance6
(   .PACKAGE_PIN(   XD[6]),
    .OUTPUT_ENABLE( oe   ),
    .D_OUT_0(       xdo[6]),
    .D_IN_0(        xdi[6]) );
SB_IO #( .PIN_TYPE(6'b 1010_01), .PULLUP(1'b 0) ) xd_instance7
(   .PACKAGE_PIN(   XD[7]),
    .OUTPUT_ENABLE( oe   ),
    .D_OUT_0(       xdo[7]),
    .D_IN_0(        xdi[7]) );

SB_IO #( .PIN_TYPE(6'b 1010_01), .PULLUP(1'b 0) ) xd_instance8
(   .PACKAGE_PIN(   XD[8]),
    .OUTPUT_ENABLE( oe   ),
    .D_OUT_0(      xdo[8]),
    .D_IN_0(       xdi[8]) );
SB_IO #( .PIN_TYPE(6'b 1010_01), .PULLUP(1'b 0) ) xd_instance9
(   .PACKAGE_PIN(   XD[9]),
    .OUTPUT_ENABLE( oe   ),
    .D_OUT_0(      xdo[9]),
    .D_IN_0(       xdi[9]) );
SB_IO #( .PIN_TYPE(6'b 1010_01), .PULLUP(1'b 0) ) xd_instance10
(   .PACKAGE_PIN(   XD[10]),
    .OUTPUT_ENABLE( oe   ),
    .D_OUT_0(       xdo[10]),
    .D_IN_0(        xdi[10]) );
SB_IO #( .PIN_TYPE(6'b 1010_01), .PULLUP(1'b 0) ) xd_instance11
(   .PACKAGE_PIN(   XD[11]),
    .OUTPUT_ENABLE( oe   ),
    .D_OUT_0(      xdo[11]),
    .D_IN_0(       xdi[11]) );
SB_IO #( .PIN_TYPE(6'b 1010_01), .PULLUP(1'b 0) ) xd_instance12
(   .PACKAGE_PIN(   XD[12]),
    .OUTPUT_ENABLE( oe   ),
    .D_OUT_0(      xdo[12]),
    .D_IN_0(       xdi[12]) );
SB_IO #( .PIN_TYPE(6'b 1010_01), .PULLUP(1'b 0) ) xd_instance13
(   .PACKAGE_PIN(   XD[13]),
    .OUTPUT_ENABLE( oe   ),
    .D_OUT_0(      xdo[13]),
    .D_IN_0(       xdi[13]) );
SB_IO #( .PIN_TYPE(6'b 1010_01), .PULLUP(1'b 0) ) xd_instance14
(   .PACKAGE_PIN(   XD[14]),
    .OUTPUT_ENABLE( oe   ),
    .D_OUT_0(      xdo[14]),
    .D_IN_0(       xdi[14]) );
SB_IO #( .PIN_TYPE(6'b 1010_01), .PULLUP(1'b 0) ) xd_instance15
(   .PACKAGE_PIN(   XD[15]),
    .OUTPUT_ENABLE( oe   ),
    .D_OUT_0(      xdo[15]),
    .D_IN_0(       xdi[15]) );

endmodule


