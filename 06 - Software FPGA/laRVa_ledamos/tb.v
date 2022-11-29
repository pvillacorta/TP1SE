//-------------------------------------------------------------------
`include "system.v"
`define SIMULATION
`timescale 1ns/10ps

module tb();

//-- Registros con señales de entrada
reg clk;
reg resetb;
reg rxd;
wire txd;

//-- Instanciamos 

SYSTEM sys1(	.clk(clk),		// Main clock input 25MHz
	.reset(~resetb),
	.rxd2(rxd)
);

// Reloj periódico
always #10 clk=~clk; // 10 ns 
//always #5 clk=~clk;

//-- Proceso al inicio
initial begin
	//-- Fichero donde almacenar los resultados
	$dumpfile("tb.vcd");
	$dumpvars(0, tb);

	resetb = 0; clk=0; rxd=1;

	#77		resetb=1;
	#10000  rxd=0;	//START
	
	#1560   rxd=1;
	#1560   rxd=0;
	#1560   rxd=0;
	#1560   rxd=0;

	#1560   rxd=0;
	#1560   rxd=0;
	#1560   rxd=1;
	#1560   rxd=0;

	#1560   rxd=1;	//STOP
	
	# 319 $display("FIN de la simulacion");
	# 300000 $finish;
	//# 1000 $finish;
end



endmodule


