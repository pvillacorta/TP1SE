SHELL = C:/Windows/System32/cmd.exe
DIR = C:/iverilog/bin
DIRGTK = C:/iverilog/gtkwave/bin

sim:	tb.v
		$(DIR)/iverilog testbench_prueba.v
		$(DIR)/vvp prueba_uart.out
		$(DIRGTK)/gtkwave UART_wave.gtkw
		