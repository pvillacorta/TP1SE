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


#define SPIDAT	 (*(volatile uint32_t*)0xE0000020) //ICE_SPI DATA
#define SPICTL	 (*(volatile uint32_t*)0xE0000024) //ICE_SPI CONTROL
#define SPISTA	 (*(volatile uint32_t*)0xE0000024) //ICE_SPI STATE
#define SPISS	 (*(volatile uint32_t*)0xE0000028) //ICE_SPI SLAVE SELECT

#define TCNT     (*(volatile uint32_t*)0xE0000060) //TIMER COUNTER REGISTER

#define GPOUT  	 (*(volatile uint8_t*)0xE0000080) 	// GPOUT
#define GPIN  	 (*(volatile uint8_t*)0xE0000084) 	// GPIN

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
#define CCLK (18000000)			// 18 MHz Frec de Reloj
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
volatile uint8_t rdix0,wrix0; // Punteros de lectura y escritura (unsigned char, otra notacion que viene en el include, 8 bit)
//FIFO UART 1:
uint8_t udat1[128]; //FIFO de recepcion para la UART0 (tamaño 32 bits)
volatile uint8_t rdix1,wrix1; // Punteros de lectura y escritura (unsigned char, otra notacion que viene en el include, 8 bit)

uint8_t GPS_FF = '0'; //Full Frame Flag (GPS) 
uint8_t GPS_FRAME[80];

// -- LECTURA UART0  ---------------------------------------
uint8_t _getch() //leer de la uart0 a través de la fifo
{
	uint8_t d;
	while(rdix0==wrix0);	//fifo vacia, espera bloqueante
	
	d=udat0[rdix0++]; //leer el dato e incremento el puntero despues para colocarlo en el siguiente dato
	rdix0&=31; //direccionamiento ciruclar (mirar escritura)
	return d;
}
                     
uint8_t haschar() {return wrix0-rdix0;}

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

// ================================================================
// ----------------------- INTERRUPCIONES -------------------------
// ================================================================

void __attribute__((interrupt ("machine"))) irq0_handler() //TRAP
{
	_printf("\nTRAP at 0x%x\n",getMEPC());
}

void __attribute__((interrupt ("machine"))) irq1_handler() // UART0 RX
{
	udat0[wrix0++]=UART0DAT; // Escribe el dato en la FIFO y postincremento del puntero de escritura
	wrix0&=31;	// direccionamiento circular del puntero de escritura (es 31 porque tenemos buffer de 32) Mascara + rapido que comparacion
	// if(wrix0==32) wrix0=0;
}

void  __attribute__((interrupt ("machine"))) irq2_handler(){ //UART0 TX
	static uint8_t a=32;
	UART0DAT=a;
	if (++a>=128) a=32;
} 

void  __attribute__((interrupt ("machine"))) irq3_handler(){ //TIMER
 volatile int a;
 _puts("0");
 a = TCNT; 
} 

void __attribute__((interrupt ("machine"))) irq4_handler() // UART1 (GPS) RX
{
	if((UART1DAT=='\n') && (udat1[--wrix1]=='\r'))
		GPS_FF='1'; //Activo el Flag que me indica fin de linea
	
	udat1[wrix1++]=UART1DAT;
	wrix1&=127;
	
}

void  __attribute__((interrupt ("machine"))) irq5_handler(){ //UART1 (GPS) TX
	static uint8_t a=32;
	UART1DAT=a;
	if (++a>=128) a=32;
}

// ================================================================

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

// -------------
// --- UART1 ---

#define BAUD1 9600

//-> LECTURA:

uint8_t _getch1() //LEE DE LA UART1 A TRAVÉS DE LA FIFO
{
	uint8_t d;
	while(rdix1==wrix1);	//fifo vacia, espera bloqueante
	d=udat1[rdix1++]; //leer el dato e incremento el puntero despues para colocarlo en el siguiente dato
	rdix1&=127; //direccionamiento ciruclar (mirar escritura)
	return d;
}

uint8_t _getGPSFrame() //LEE DEL GPS un Frame Completo
{
	volatile uint8_t pointer=0;
	while(!GPS_FF);	//Trama Incompleta
	while(_getch1()!='$');//Busca el inicio de trama
	GPS_FRAME[pointer++]='$';
	
	do{
	GPS_FRAME[pointer++]=_getch1();
	}
	while (GPS_FRAME[(pointer-1)]!='\n');
	
	for(volatile uint8_t i=0; i<pointer ; i++){
		_putch(GPS_FRAME[i]);
	}
	_puts("\n");
	GPS_FF='0';
	return pointer;
}

void _putch2(int c) // ESCRITURA EN UART1
{
	while((UART1STA&2)==0); // Comprueba el flag THRE
	//if (c == '\n') _putch('\r');
	UART1DAT = c;
}
// -------------

// --------------------

// GPOUT 
#define ice_led1 		(0b00000001)	//GPOUT0 -> ice_led1
#define ice_led2 		(0b00000010)	//GPOUT1 -> ice_led2
#define ice_led3 		(0b00000100)	//GPOUT2 -> ice_led3
#define ice_led4 		(0b00001000)	//GPOUT3 -> ice_led4
#define STEPUP_CE 		(0b00010000)	//GPOUT4 -> STEPUP_CE
#define GAS_5V_CTRL 	(0b00100000)	//GPOUT5 -> GAS_5V_CTRL
#define GAS_1V4_CTRL 	(0b01000000)	//GPOUT6 -> GAS_1V4_CTRL
#define DUST_CTRL 		(0b10000000)	//GPOUT7 -> DUST_CTRL

// ICE_SPI 
#define BME680_CS 		(0b00000000000000000000000000000010)	//SS0 (bit0 en baja)
#define ADC_CS 			(0b00000000000000000000000000000001)	//SS1 (bit1 en baja)

// HABILITACION DE INTERRUPCIONES IRQEN:
#define IRQEN_U0RX 	(0b00000010)	//IRQ VECT 1 (1<<1)
#define IRQEN_U0TX 	(0b00000100)	//IRQ VECT 2 (1<<2)
#define IRQEN_TIMER (0b00001000)	//IRQ VECT 3 (1<<3)
#define IRQEN_U1RX	(0b00010000)	//IRQ VECT 4 (1<<4)
#define IRQEN_U1TX 	(0b00100000)	//IRQ VECT 5 (1<<5)
#define IRQEN_U2RX	(0b01000000)	//IRQ VECT 6 (1<<6)
#define IRQEN_U2TX 	(0b10000000)	//IRQ VECT 7 (1<<7)

#include "gps.c" //Rutinas de GPS (UART1)
#include "test.c" //Rutinas de test

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
	c = UART1DAT;		// Clear RX garbage
	

 
	IRQVECT0=(uint32_t)irq0_handler; //TRAP
	IRQVECT1=(uint32_t)irq1_handler; //UART0 RX
	IRQVECT2=(uint32_t)irq2_handler; //UART0 TX
	IRQVECT3=(uint32_t)irq3_handler; //Timer
	IRQVECT4=(uint32_t)irq4_handler; //UART1 RX
	IRQVECT5=(uint32_t)irq5_handler; //UART1 TX

	IRQEN = 0;
	asm volatile ("ecall");  //Salta interrupcion Software
	asm volatile ("ebreak"); //Salta interrupcion Software
	
	_puts(menutxt);
	_puts("Hola mundo\n");
	SPISS=BME680_CS;
	SPITest();
	while(1){
	}
		
	//_getGPSFrame();		
	
	//IRQEN|=IRQEN_U1RX;
	//test_U1_IRQREAD(void);
	
	
	
	
	// while (1)
	// {
			// _puts("Command [123dx]> ");
			// char cmd = _getch();
			// if (cmd > 32 && cmd < 127)
				// _putch(cmd);
			// _puts("\n");

			// switch (cmd)
			// {
			// case '1':
			    // _puts(menutxt);
				// break;
			// case '2':
				// IRQEN^=4;	// Toggle IRQ enable for UART TX
				// _delay_ms(100);
				// break;
			// case 'x':
				// _puts("Upload APP from serial port (<crtl>-F) and execute\n");
				// if(getw()!=0x66567270) break;
				// p=(uint8_t *)getw();
				// n=getw();
				// i=getw();
				// if (n) {
					// do { *p++=_getch(); } while(--n);
				// }

				// if (i>255) {
					// pcode=(void (*)())i;
					// pcode();
				// } 
				// break;
			// case 'q':
				// asm volatile ("jalr zero,zero");
			// case 't':
				// break;
			// default:
			// _puts(menutxt);
				// continue;
			// }
	// }
}
