// =======================================================================
// Proyecto Datalogger for IoT Curso 2022-2023
// Fecha: 05/12/2022 
// Autor: Pablo Villacorta, Rubén Serrano, Óscar Martín y Andrés Martín
// Asignatura: Taller de Proyectos I
// File: main.c  Programa principal
// =======================================================================

#include <stdint.h>
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef signed char  s8;
typedef signed short s16;
typedef signed int   s32;


// ==============================================================================
// -------------------------- REGISTROS MAPEADOS --------------------------------
// ==============================================================================

#define UART0DAT  (*(volatile uint8_t*)0xE0000000) 	//UART0 DATA
#define UART0STA  (*(volatile uint32_t*)0xE0000004) //UART0 STATE
#define UART0BAUD (*(volatile uint32_t*)0xE0000004) //UART0 BAUD DIVIDER

#define UART1DAT  (*(volatile uint8_t*)0xE0000008) 	//UART1 DATA
#define UART1STA  (*(volatile uint32_t*)0xE000000C) //UART1 STATE
#define UART1BAUD (*(volatile uint32_t*)0xE000000C) //UART1 BAUD DIVIDER

#define UART2DAT  (*(volatile uint8_t*)0xE0000010)  //UART2 DATA
#define UART2STA  (*(volatile uint32_t*)0xE0000014) //UART2 STATE
#define UART2BAUD (*(volatile uint32_t*)0xE0000014) //UART2 BAUD DIVIDER


#define SPIDAT	 (*(volatile uint32_t*)0xE0000020) //SPI DATA
#define SPICTL	 (*(volatile uint32_t*)0xE0000024) //SPI CONTROL
#define SPISTA	 (*(volatile uint32_t*)0xE0000024) //SPI STATE
#define SPISS	 (*(volatile uint32_t*)0xE0000028) //SPI SLAVE SELECT

#define TCNT     (*(volatile uint32_t*)0xE0000060) //TIMER COUNTER REGISTER

#define IRQEN	 (*(volatile uint32_t*)0xE00000C0) //INTERRUPT ENABLE
#define IRQVECT0 (*(volatile uint32_t*)0xE00000E0) //TRAP
#define IRQVECT1 (*(volatile uint32_t*)0xE00000E4) //RX0
#define IRQVECT2 (*(volatile uint32_t*)0xE00000E8) //TX0
#define IRQVECT3 (*(volatile uint32_t*)0xE00000EC) //TIMER
#define IRQVECT4 (*(volatile uint32_t*)0xE00000F0) //RX1
#define IRQVECT5 (*(volatile uint32_t*)0xE00000F4) //TX1
#define IRQVECT6 (*(volatile uint32_t*)0xE00000F8) //RX2
#define IRQVECT7 (*(volatile uint32_t*)0xE00000FC) //TX2

void delay_loop(uint32_t val);	// (3 + 3*val) cycles
#define CCLK (18000000)
#define _delay_us(n) delay_loop((n*(CCLK/1000)-3000)/3000)
#define _delay_ms(n) delay_loop((n*(CCLK/1000)-30)/3)


void _putch(int c)
{
	while((UART0STA&2)==0); // When THRE = 0 (Uart0) [Espera a que no este ocupado el THR]
	//if (c == '\n') _putch('\r');
	UART0DAT = c;	//Escribo el dato entero a transmitir (4bytes)
}

void _puts(const char *p)
{
	while (*p)
		_putch(*(p++));
}
/*
uint8_t _getch()
{
	while((UART0STA&1)==0);
	return UART0DAT;
}

uint8_t haschar() {return UART0STA&1;}
*/

#define putchar(d) _putch(d)
#include "printf.c"

const static char *menutxt="\n"
"\n\n"
"888                8888888b.  888     888\n"         
"888                888   Y88b 888     888\n"        
"888                888    888 888     888\n"       
"888        8888b.  888   d88P Y88b   d88P 8888b.\n"  
"888           '88b 8888888P'   Y88b d88P     '88b\n"
"888       .d888888 888 T88b     Y88o88P  .d888888\n" 
"888888888 888  888 888  T88b     Y888P   888  888\n"
"888888888 'Y888888 888   T88b     Y8P    'Y888888\n"
"\nIts Alive :-)\n"
"\n";             

//FIFO UART 0:
uint8_t udat0[32]; //FIFO de recepcion para la UART0 (tamaño 32 bits)
volatile uint8_t rdix,wrix; // Punter (unsigned char, otra notacion que viene en el include, 8 bit)

// -- LECTURA UART0  ---------------------------------------
uint8_t _getch() //leer de la uart0 a través de la fifo
{
	uint8_t d;
	while(rdix==wrix);	//fifo vacia, espera bloqueante
	d=udat0[rdix++]; //leer el dato e incremento el puntero despues para colocarlo en el siguiente dato
	rdix&=31; //direccionamiento ciruclar (mirar escritura)
	return d;
}
                     
uint8_t haschar() {return wrix-rdix;}

uint32_t __attribute__((naked)) getMEPC() //Funcion que devuelve el PC
{
	asm volatile(
	//"	csrrw	a0,0x341,zero	\n"
	//"	csrrw	zero,0x341,a0	\n"
	"	.word	0x34101573 		\n"
	"	.word   0x34151073		\n"
	"	ret						\n"
	);
}
// -- Interrupciones ---------------------------------------

void __attribute__((interrupt ("machine"))) irq1_handler() //TRAP
{
	_printf("\nTRAP at 0x%x\n",getMEPC());
}

void __attribute__((interrupt ("machine"))) irq2_handler() //RX0 cada vez que llega un dato
{
	udat0[wrix++]=UART0DAT; // Escribe el dato en la FIFO y postincremento del puntero de escritura
	wrix&=31;	// direccionamiento circular del puntero de escritura (es 31 porque tenemos buffer de 32) Mascara + rapido que comparacion
	// if(wrix==32) wrix=0;
}

void  __attribute__((interrupt ("machine"))) irq3_handler(){ //TX0
	static uint8_t a=32;
	UART0DAT=a;
	if (++a>=128) a=32;
}


// --------------------------------------------------------

uint32_t spixfer (uint32_t d)
{
	SPIDAT=d;
	while(SPISTA&1);
	return SPIDAT;
}

// --------------------------------------------------------
#define NULL ((void *)0)

// --- UART0 ---

#define BAUD0 115200

uint32_t getw()
{
	uint32_t i;
	i=_getch();
	i|=_getch()<<8;
	i|=_getch()<<16;
	i|=_getch()<<24;
	return i;
}

uint8_t *_memcpy(uint8_t *pdst, uint8_t *psrc, uint32_t nb)
{
	if (nb) do {*pdst++=*psrc++; } while (--nb);
	return pdst;
}
// --------------------

// --- UART1 ---

#define BAUD1 9600

uint8_t _getch1() // LEE DE UART1
{
	while((UART1STA&1)==0); // Comprueba el flag DV (Si esta a 0 se queda esperando al dato)
	return UART1DAT;
}

void _putch2(int c) // ESCRITURA EN UART1
{
	while((UART1STA&2)==0); // Comprueba el flag THRE
	//if (c == '\n') _putch('\r');
	UART1DAT = c;
}
// --------------------


// ==============================================================================
// ------------------------------------ MAIN ------------------------------------
// ==============================================================================
void main()
{
	char c,buf[17];
	uint8_t *p;
	unsigned int i,j;
	int n;
	void (*pcode)();
	uint32_t *pi;
	uint16_t *ps;
	
	UART0BAUD=(CCLK+BAUD0/2)/BAUD0 -1;	
	UART1BAUD = (CCLK+BAUD1/2)/BAUD1 -1;
	
	_delay_ms(100);
	c = UART0DAT;		// Clear RX garbage
	IRQVECT0=(uint32_t)irq1_handler; //TRAP
	IRQVECT1=(uint32_t)irq2_handler; //UART0 RX
	IRQVECT2=(uint32_t)irq3_handler; //UART0 TX


	IRQEN=1<<1;			// Enable UART0 RX IRQ (bit 1 de Interrupt Enable)

	asm volatile ("ecall");
	asm volatile ("ebreak");
	_puts(menutxt);
	_puts("Hola mundo\n");
	
	//while(1){ // PRUEBA DE ESCRITURA DESDE LA UART0
	//_putch('A');
	//}
	
	while(1){ // PRUEBA DE LECTURA DESDE LA NUEVA UART1
	char uart1_data = _getch1();
	_putch(uart1_data);
	}
	
	while (1)
	{
			_puts("Command [123dx]> ");
			char cmd = _getch();
			if (cmd > 32 && cmd < 127)
				_putch(cmd);
			_puts("\n");

			switch (cmd)
			{
			case '1':
			    _puts(menutxt);
				break;
			case '2':
				IRQEN^=4;	// Toggle IRQ enable for UART TX
				_delay_ms(100);
				break;
			case 'x':
				_puts("Upload APP from serial port (<crtl>-F) and execute\n");
				if(getw()!=0x66567270) break;
				p=(uint8_t *)getw();
				n=getw();
				i=getw();
				if (n) {
					do { *p++=_getch(); } while(--n);
				}

				if (i>255) {
					pcode=(void (*)())i;
					pcode();
				} 
				break;
			case 'q':
				asm volatile ("jalr zero,zero");
			case 't':
				break;
			default:
				continue;
			}
	}
}
