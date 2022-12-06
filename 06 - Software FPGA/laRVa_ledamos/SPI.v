// =======================================================================
// Proyecto Datalogger for IoT Curso 2022-2023
// Fecha: 1/12/2022 
// Autor: Pablo Villacorta, Rubén Serrano, Óscar Martín y Andrés Martín
// Asignatura: Taller de Proyectos I
// File: SPI.v  Fichero para la creación del SPI
// =======================================================================


module reg_desp(input shift, input load, input serial_in,
                input clk, input [31:0] d, output [31:0] q);
  reg [31:0] desp;    
  always @(posedge clk)
      begin
        if (load)
          desp <= d;
        else if (shift)
          desp <= {desp[30:0],serial_in};
      end  
  assign q = desp;
endmodule

module counter(input load,input clk,input [7:0] Divider,
              output zero);
    reg count; 
    always @(posedge clk)
      begin
  	count <= load ? Divider : count-1; //Si la entrada de load esta activada se vuelve a contar
  	zero <= count ? 1 : 0;
      end
endmodule

module bit_cnt(input load,input clk,input count,input [5:0] DLEN,
               output zero); //ESTE LO DEJO PARA EL FINAL QUE NO LO TENGO CLARO
    reg bit_cnt; 
    always @(posedge clk)
      begin
  	bit_cnt <= load ? DLEN : bit_cnt-1; //Si la entrada de load esta activada se vuelve a contar
  	zero <= bit_cnt ? 1 : 0;
      end
  
  
endmodule

module shifter(input shift, input load, input clk, input serial_in,
               input [31:0] paralel_in, output mosi, output [31:0] RX_data);
  reg [31:0] rx;
  reg_desp registro_desplazamiento (shift,load,serial_in,clk,paralel_in,rx);
  
  //A VER QUE TENEMOS QUE SEGUIR PONIENDO AQUI
  
  
  //se le meten dos entradas? quiero decir, parece que a la entrada paralel_in
  //se le introduce TX_DATA y DLEN?? y que es DLEN??
  
endmodule

module divisor_freq(input clk, input load, input [7:0] cdo,
                   input n_stop, output falling, output SCLK);
  reg zero;
  reg load_div;
  wire T;
  assign load_div = (load | zero); //con load + zero deja, pero no se si se puede
  counter contador (load_div, clk, cdo, zero);
  assign T = (zero & n_stop);
  always @(posedge clk)
    SCLK<=~T;
  assign falling = SCLK & T;
  
endmodule


module SPI_MASTER(
	input clk, 
	input miso, 
	input wr,
	input [31:0] din, 
	input [7:0] divider, 
	input [5:0] bits,
	output sck, 
	output mosi, 
	output busy, 
	output [31:0] dout
	); 
  
endmodule
