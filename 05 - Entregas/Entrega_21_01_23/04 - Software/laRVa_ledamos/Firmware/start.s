# 1 "Firmware/start.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "Firmware/start.S"

##############################################################################
# RESET & IRQ
##############################################################################

 .global main, irq1_handler, irq2_handler, irq3_handler

 .section .boot
reset_vec:
 j start
.section .text

######################################
### Main program
######################################

start:

 li sp,16384

# copy data section
 la a0, _sdata
 la a1, _sdata_values
 la a2, _edata
 bge a0, a2, end_init_data
loop_init_data:
 lw a3,0(a1)
 sw a3,0(a0)
 addi a0,a0,4
 addi a1,a1,4
 blt a0, a2, loop_init_data
end_init_data:
# zero-init bss section
 la a0, _sbss
 la a1, _ebss
 bge a0, a1, end_init_bss
loop_init_bss:
 sw zero, 0(a0)
 addi a0, a0, 4
 blt a0, a1, loop_init_bss
end_init_bss:
# call main
 call main
loop:
 j loop

 .globl delay_loop
delay_loop:
 addi a0,a0,-1
 bnez a0, delay_loop
 ret
