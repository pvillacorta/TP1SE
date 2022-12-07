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
#--- Code PRAO_START:

# lui a0,	0xE0000
# addi a0,a0,+128
# #addi a1, zero,+3591	#Cargo en a1 el numero
# addi a1, zero, +1543
# sw a1,	4(a0)		#En la direccion base que esta en a0 + 128, es decir, 0xE0000080 // Meto en el divider de la UART 0 el dato

# addi a1, zero, +0x41	#Cargo en a1 el numero 0x41 -> es una A
# sw a1,	0(a0)			#Comienza la tx, en la dirección 0xE0000080 // Debería poder ver como sale el 41

# nollega:	#Voy a comprobar que la recepción del dato también es correcta
# #la manera correcta es antes de leer nada, revisar los flags
# lw a2, 4(a0)		#Estoy leyendo en la uart + 4 (donde estan los flags) y lo guardo en a2	
# andi a2,a2,+1	#Aplico la mascara todo ceros y un 1 (Depende si dv vale 0 o dv vale 1)
# beqz a2,nollega	#salta 2 instrucciones atrás (8 dir) // Mientras no llegue el dato me quedo para siempre
# #cuando llegue el dato el flag dv vale 1
# lw a2, 0(a0)	#Hago un load de lo que hay en la base a a2 para verlo

# end:
# j end	#para que solo ejecute lo mio


 # #habilitar interrupciones
 # #0xE00000C0
 # lui a0,	0xE0000
 # #cargar un 8 en el enable
 # addi a2,a0,+192
 # addi a1, zero,+8	#Cargo en a1 el numero
 # sw a1,0(a2)
 
 # addi a0,a0,+96
 # addi a1, zero,+3	#Cargo en a1 el numero
 # sw a1,0(a0)
 
 # end:
 # nop
 # addi a0,a0,+0
 # lw a3, 0(a0) #Leo el contador (lo que hay en a0)
 # j end	#para que solo ejecute lo mio
# #--- Code PRAO_END






 li sp,8192

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
