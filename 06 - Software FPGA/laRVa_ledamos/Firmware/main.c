// =======================================================================
// Proyecto Datalogger for IoT Curso 2022-2023
// Fecha: 26/01/2023 
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

#define SPILDAT	 (*(volatile uint32_t*)0xE0000030) //LORA_SPI DATA
#define SPILCTL	 (*(volatile uint32_t*)0xE0000034) //LORA_SPI CONTROL
#define SPILSTA	 (*(volatile uint32_t*)0xE0000034) //LORA_SPI STATE
#define SPILSS	 (*(volatile uint32_t*)0xE0000038) //LORA_SPI SLAVE SELECT

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

// GPOUT 
#define ice_led1 		(0b00000001)	//GPOUT0 -> ice_led1
#define ice_led2 		(0b00000010)	//GPOUT1 -> ice_led2
#define ice_led3 		(0b00000100)	//GPOUT2 -> ice_led3
#define ice_led4 		(0b00001000)	//GPOUT3 -> ice_led4
#define STEPUP_CE 		(0b00010000)	//GPOUT4 -> STEPUP_CE
#define GAS_5V_CTRL 	(0b00100000)	//GPOUT5 -> GAS_5V_CTRL
#define GAS_1V4_CTRL 	(0b01000000)	//GPOUT6 -> GAS_1V4_CTRL
#define DUST_CTRL 		(0b10000000)	//GPOUT7 -> DUST_CTRL 

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

uint8_t _sizeof(char *string){ // Tamaño de un array
	uint8_t size = 1;
	while((*string++)!='\0'){
		size++;
	}
	return size;
}

#define putchar(d) _putch(d)
#include "printf.c"

const static char *menutxt="\n"
"\n\n"
"8888888888  8888888b.   8888888888  8888888888\n"         
"888    888  888   Y88b  888    888  888    888\n"        
"888    888  888    888  888    888  888    888\n"       
"8888888888  888   d88P  888    888  888    888\n"  
"888         8888888P    8888888888  888    888\n"
"888         888 T88b    888    888  888    888\n" 
"888         888  T88b   888    888  888    888\n"
"888         888   T88b  888    888  8888888888\n"
"\nIts Alive :-)\n"
"\n";  

const static char *gpoutTxt="-- GPOUT ASSIGNMENTS: --\n"
"GPOUT[0] -> ice_led1\n"
"GPOUT[1] -> ice_led2\n"
"GPOUT[2] -> ice_led3\n"
"GPOUT[3] -> ice_led4\n"
"GPOUT[4] -> STEPUP_CE\n"
"GPOUT[5] -> GAS_5V_CTRL\n"
"GPOUT[6] -> GAS_1V4_CTRL\n"
"GPOUT[7] -> DUST_CTRL\n"
"-------------------------\n";         

//FIFO UART 0:
uint8_t udat0[32]; //FIFO de recepcion para la UART0 (tamaño 32 bits)
volatile uint8_t rdix0,wrix0; // Punteros de lectura y escritura (unsigned char, otra notacion que viene en el include, 8 bit)
//FIFO UART 1 (GPS)
uint8_t udat1[128]; //FIFO de recepcion para la UART0 (tamaño 32 bits)
volatile uint8_t rdix1,wrix1; // Punteros de lectura y escritura (unsigned char, otra notacion que viene en el include, 8 bit)

//FIFO UART 2 (GPS)
uint8_t udat2[128]; //FIFO de recepcion para la UART0 (tamaño 32 bits)
volatile uint8_t rdix2,wrix2; // Punteros de lectura y escritura (unsigned char, otra notacion que viene en el include, 8 bit)

uint8_t GPS_FF = '0'; //Full Frame Flag (GPS) 
uint8_t GPS_FRAME[80];

// Timer
uint8_t clkMode = 0;
uint8_t binaryCount = 0;

// GPS
uint8_t volcarOutputGPS=0;

// SensorAnalogico GAS:
uint16_t Ch4LPGValue=0;
uint16_t COValue=0;
uint16_t polvoValue=0; 

// Variables BME
uint8_t temp = 50;
uint16_t presion = 0;
uint8_t humedad = 0;

// Variables GPS
char UTC_time[10]="";
char date[10]="";

// -- LECTURA UART0  ---------------------------------------
uint8_t _getch() //leer de la uart0 a través de la fifo
{
	uint8_t d;
	while(rdix0==wrix0){}	//fifo vacia, espera bloqueante
	
	d=udat0[rdix0++]; //leer el dato e incremento el puntero despues para colocarlo en el siguiente dato
	rdix0&=31; //direccionamiento ciruclar (mirar escritura)
	return d;
}
                     
uint8_t haschar() {return wrix0-rdix0;}

// --------------------------------------------------------
// Return PC
uint32_t __attribute__((naked)) getMEPC()
{
	asm volatile(
	//"	csrrw	a0,0x341,zero	\n"
	//"	csrrw	zero,0x341,a0	\n"
	"	.word	0x34101573 		\n"
	"	.word   0x34151073		\n"
	"	ret						\n"
	);
}
    
// --------------------------------------------------------
// Print Byte in binary & hex

void _printfBin(uint8_t byte){
	_printf("%d%d%d%d %d%d%d%d || %x\n",
	(byte & 0x80 ? 1 : 0),  
	(byte & 0x40 ? 1 : 0), 
	(byte & 0x20 ? 1 : 0),  
	(byte & 0x10 ? 1 : 0), 
	(byte & 0x08 ? 1 : 0), 
	(byte & 0x04 ? 1 : 0),  
	(byte & 0x02 ? 1 : 0), 
	(byte & 0x01 ? 1 : 0),
	byte   
	);
}
// --------------------

// ITOA -> Integer to String -------------------------------
void my_itoa(long i, char *string)
{
	int power = 0, j = 0;
	j = i;
	for (power = 1; j>10; j /= 10)
		power *= 10;
	for (; power>0; power /= 10){
		*string++ = '0' + i / power;
		i %= power;
	}
	*string = '\0';
}
//----------------------------------------------------------

 
// ================================================================
// ----------------------- INTERRUPCIONES -------------------------
// ================================================================

// HABILITACION DE INTERRUPCIONES IRQEN:
#define IRQEN_U0RX 	(0b00000010)	//IRQ VECT 1 (1<<1)
#define IRQEN_U0TX 	(0b00000100)	//IRQ VECT 2 (1<<2)
#define IRQEN_TIMER (0b00001000)	//IRQ VECT 3 (1<<3)
#define IRQEN_U1RX	(0b00010000)	//IRQ VECT 4 (1<<4)
#define IRQEN_U1TX 	(0b00100000)	//IRQ VECT 5 (1<<5)
#define IRQEN_U2RX	(0b01000000)	//IRQ VECT 6 (1<<6)
#define IRQEN_U2TX 	(0b10000000)	//IRQ VECT 7 (1<<7)

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
	 switch (clkMode)
	{
		case 0:	// (0) LED BLINK MODE
			GPOUT = (GPOUT & 0b11110000)+binaryCount;
			binaryCount=binaryCount+1;
			if(binaryCount==0b00010000) //Cuenta hasta 16 (0 to 15)
				binaryCount=0;
			break;
	
		case 1:	// (1) GAS SENSOR MODE 5V Cicle -> Cuando salta activo 5V
			//Muestrear el final de 1v4
			COValue=((ReadADC(CMD_CH0)*3300)>>10);
			GPOUT = (STEPUP_CE|GAS_5V_CTRL|ice_led2); //Activa 5V Control
			
			clkMode=2;		// Para que cuando salte el reloj pase a Modo 1V4V
			TCNT=(60*CCLK); //Configuramos el reloj para los 60 seg en 5V
			break;
		
		case 2:	// (2) GAS SENSOR MODE 1V4 Cicle -> Cuando salta activo 1v4
			//Muestrear el final de 5v
			Ch4LPGValue=((ReadADC(CMD_CH0)*3300)>>10);
			
			GPOUT = (STEPUP_CE|GAS_1V4_CTRL|ice_led1); //Activa 1V4 CTRL 
			
			clkMode=1;		// Para que cuando salte el reloj pase a Modo 5V
			TCNT=(90*CCLK); //Configuramos el reloj para los 90 seg en 1V4
			break;
		
		case 3:	// (3) Sensor de Polvo
			GPOUT |= DUST_CTRL; //Activa Dust Control y el led 4
			_delay_ms(0.28); //Delay 0,28 mseg 
			polvoValue=((ReadADC(CMD_CH1)*3300)>>10); //Muestreo en mV del CAD
			
			_delay_ms(0.4); // Delay 0,4 mseg y bajo el pulso
			GPOUT ^= DUST_CTRL; //Desactiva dust control
			break;
		
		default: 
			_puts("Clk Mode Error");
			break;
	}
	
 a = TCNT; 
} 
   
void __attribute__((interrupt ("machine"))) irq4_handler() // UART1 (GPS) RX
{	
	// (1) Modo Volcar
	if(volcarOutputGPS==1){
		_putch(UART1DAT);
	}
	// (2) Modo Decodificar tramas
	else{
		if((UART1DAT=='\r'))
			GPS_FF='1'; //Activo el Flag que me indica fin de linea
	}
	
	udat1[wrix1++]=UART1DAT;
	wrix1&=127;
} 

void  __attribute__((interrupt ("machine"))) irq5_handler(){ //UART1 (GPS) TX
	static uint8_t a=32;
	UART1DAT=a;
	if (++a>=128) a=32;
}

void __attribute__((interrupt ("machine"))) irq6_handler() // UART2 RX
{	
	udat2[wrix2++]=UART2DAT;
	_putch(UART2DAT);
	wrix2&=127;
} 

void  __attribute__((interrupt ("machine"))) irq7_handler(){ //UART2 TX
	static uint8_t a=32;
	UART2DAT=a;
	if (++a>=128) a=32;
}
 
// ================================================================

#define NULL ((void *)0)

// -------------
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


char* cnc(char* str1, char* str2){
	return _memcpy(_memcpy(str1, str1,_sizeof(str1)) - 1, str2, _sizeof(str2)) -1;
}


 
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

void _putch2(int c) // ESCRITURA EN UART1
{
	while((UART1STA&2)==0); // Comprueba el flag THRE
	//if (c == '\n') _putch('\r');
	UART1DAT = c;
} 

// -------------
// --- UART2 --- 

#define BAUD2 9600


// -------------

#include "spiSensors.c" //Rutinas de test
#include "spiLoRA.c" //Rutinas de test 
#include "gps.c" //Rutinas de GPS (UART1)
#include "gpin.c" //Rutinas de GPIN
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
	
	UART0BAUD = (CCLK+BAUD0/2)/BAUD0 -1;	
	UART1BAUD = (CCLK+BAUD1/2)/BAUD1 -1;
	UART2BAUD = (CCLK+BAUD2/2)/BAUD2 -1;
	_delay_ms(100);
	c = UART0DAT;		// Clear RX garbage
	c = UART1DAT;		// Clear RX garbage
	c = UART2DAT;		// Clear RX garbage
	 
	IRQVECT0=(uint32_t)irq0_handler; //TRAP
	IRQVECT1=(uint32_t)irq1_handler; //UART0 RX 
	IRQVECT2=(uint32_t)irq2_handler; //UART0 TX
	IRQVECT3=(uint32_t)irq3_handler; //Timer
	IRQVECT4=(uint32_t)irq4_handler; //UART1 RX
	IRQVECT5=(uint32_t)irq5_handler; //UART1 TX
	IRQVECT6=(uint32_t)irq6_handler; //UART2 RX
	IRQVECT7=(uint32_t)irq7_handler; //UART2 TX

	test();
}
