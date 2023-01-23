// =======================================================================
// Proyecto Datalogger for IoT Curso 2022-2023
// Fecha: 17/01/2022 
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

uint8_t GPS_FF = '0'; //Full Frame Flag (GPS) 
uint8_t GPS_FRAME[80];

// Timer
uint8_t clkMode = 0;
uint8_t binaryCount = 0;

// GPS
uint8_t volcarOutputGPS=0;

// SensorAnalogico GAS:
uint32_t gasValue1v4=0;
uint32_t gasValue5v=0;
uint32_t polvoValue=0;


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

#include "spiSensors.c" //Rutinas de test
 
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
		gasValue1v4=ReadADC(CMD_CH0);
		GPOUT = (STEPUP_CE|GAS_5V_CTRL|ice_led2); //Activa 5V Control
		
		clkMode=2;		// Para que cuando salte el reloj pase a Modo 1V4V
		TCNT=(60*CCLK); //Configuramos el reloj para los 60 seg en 5V
		break;
		
		case 2:	// (2) GAS SENSOR MODE 1V4 Cicle -> Cuando salta activo 1v4
		//Muestrear el final de 5v
		gasValue5v=ReadADC(CMD_CH0);
		GPOUT = (STEPUP_CE|GAS_1V4_CTRL|ice_led1); //Activa 1V4 CTRL 
		
		clkMode=1;		// Para que cuando salte el reloj pase a Modo 5V
		TCNT=(90*CCLK); //Configuramos el reloj para los 90 seg en 1V4
		break;
		
		case 3:	// (3) Sensor de Polvo
		
		GPOUT |= DUST_CTRL; //Activa Dust Control y el led 4
		_delay_ms(0.28); //Delay 0,28 mseg
		polvoValue=ReadADC(CMD_CH1); //Muestreo
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
	_putch('T');
	UART1DAT=a;
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

#include "gps.c" //Rutinas de GPS (UART1)
#include "test.c" //Rutinas de test
#include "spiLoRA.c" //Rutinas de test 
#include "gpin.c" //Rutinas de GPIN 
  
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
	GPOUT = 0; //Inicializa el GPOUT a 0
	
	_puts(menutxt);     
	_puts("Hola mundo\n");   

	asm volatile ("ecall");  //Salta interrupcion Software
	asm volatile ("ebreak"); //Salta interrupcion Software
	
	SPICTL = (8<<8)|8;  // Define Registro control SPI 0 (BME y ADC)
	startBME680(); //Programa los registros de configuracion
	
	SPILCTL = (8<<8)|8; // Define Registro control SPI 1 (LoRa)
	  
	IRQEN = IRQEN_TIMER;
	//TCNT=CCLK; //Configuramos el reloj cada segundo
	
	
while (1)
	 {
			IRQEN |= IRQEN_U0RX;
			_puts("\n--- TEST ---\n");
			_puts("-> z: Lee los registros del transceptor LoRa\n");
			_puts("-> 5: Lee los registros del sensor BME680\n");
			_puts("-> 4: Lee los canales del ADC\n");
			_puts("-> 6: Activa/desactiva STEPUP_CE, bit gpout[4]\n");
			_puts("-> 7: Activa/desactiva DUST_CTRL, bit gpout[7]\n");
			_puts("-> 8: Activa/desactiva GAS_1V4_CTRL, bit gpout[6]\n");
			_puts("-> 9: Activa/desactiva GAS_5V_CTRL, bit gpout[5]\n");
			_puts("- 0: Lee salida del sensor de particulas\n");
			_puts("- r: Lee el Sensor Presion MS86072BA01 (I2C)\n");			
			_puts("-> q: Salta a la direccion 0 (casi como un reset)\n");			
			_puts("-> t: Prueba el temporizador de los LED (T = 0.5 seg, al arrancar 1 seg) [BLOQ]\n");
			_puts("-> g: Decodificar Trama GPS (Espera trama)\n");
			_puts("-> h: La salida del GPS (UART1) a la UART0 [BLOQ]\n");
			_puts("- k: La salida de la UART2 a la UART0\n");
			_puts("-> l: Lee estado del TIMER\n");
			_puts("-> 1: Pinta menu por UART0\n");
			_puts("- 2: Envia datos por UART0 via interrupciones \n");
			_puts("-> 3: Lectura GPIN \n");
			_puts("-> G: Activar sensor GAS \n");
			_puts("-> P: Activar sensor Polvo \n");
			_puts("- L: Transmitir datos por LoRA \n\n");
			
			_puts("Command [z4567890rqtgkl123GPL]> ");
			char cmd = _getch();
			if (cmd > 32 && cmd < 127)				
				_putch(cmd);
				_puts("\n");
 
			switch (cmd)
			{
			case 'z': //Lee los registros del transceptor LoRa
				readAllLoRaRegs();
				printLoRaRegs();   
				break;
			case '5': //Lee los registros del sensor BME680
				readAllBMERegs();
				printBMERegs();
				measureBME680();
				break;
			case '4': //Lee los canales del ADC
				printAdcChannels();
				break;
		 
		 	case '6': //Activa/desactiva STEPUP_CE
				GPOUT^= STEPUP_CE;
				_puts("GPOUT = ");
				_printfBin(GPOUT);
				_puts(gpoutTxt);
				break;
			case '7': //Activa/desactiva DUST_CTRL
				GPOUT^= DUST_CTRL;
				_puts("GPOUT = ");
				_printfBin(GPOUT);
				_puts(gpoutTxt);
				break;
			case '8': //Activa/desactiva GAS_1V4_CTRL
				GPOUT^= GAS_1V4_CTRL;
				_puts("GPOUT = ");
				_printfBin(GPOUT);
				_puts(gpoutTxt);
				
				break;
			case '9': //Activa/desactiva GAS_5V_CTRL
				GPOUT^= GAS_5V_CTRL;
				_puts("GPOUT = ");
				_printfBin(GPOUT);
				_puts(gpoutTxt);
				break;
			case '0': //Lee salida del sensor de partículas
				_puts("Lo siento aun no hemos implementado esto :)");
				break;
			case 'r': //Lee el Sensor Presión MS86072BA01 (I2C)
				_puts("Lo siento aun no hemos implementado esto :)");
				break;  
			case 'q': //Salta a la dirección 0 (casi como un reset)
				asm volatile ("jalr zero,zero");
				break;
			case 't': //Prueba el temporizador de los LED (periodo 0.5 segundos, al arrancar 1 segundo)
				_puts("Secuencia Iniciada");
				clkMode=0;
				IRQEN |= IRQEN_TIMER; //Habilito interrupciones del temporizador y deshabilito UART0 (Ya que tiene prioridad)
				TCNT=CCLK>>1; // Periodo de 1/2 seg
				 
				break;
			case 'g': //Imprimir GPS Decodificado
				volcarOutputGPS=0;
				IRQEN=IRQEN_U1RX;   
				getGPSFrame();
				break;
			case 'h': //La salida del GPS (UART1) a la UART0
				volcarOutputGPS=1;
				IRQEN=IRQEN_U1RX;   
				while(1){
					_delay_ms(1);
				} // Bloqueante
				
				break;				
			case 'k': //La salida de la UART2 a la UART0
				_puts("Lo siento aun no hemos implementado esto :)");
				break;	
			case 'l': //Lee estado del TIMER
				_printf("\nTCNT: %d\n",TCNT);
				break;						
			case '1': //Pinta menú por UART0
			    _puts(menutxt);
				break;
			case '2': //Envía datos por UART0 vía interrupciones
				_puts("Lo siento aun no hemos implementado esto :)");
				break;  
			case '3': //Lectura del GPIN
				GpinRead();
				break;
			case 'G': //Activar Sensor GAS
				IRQEN |= IRQEN_TIMER;
				ReadGAS();
				break;	
			case 'P': //Activar Sensor Polvo
				IRQEN |= IRQEN_TIMER;
				ReadDust();
			break;	
								
			case 'x':
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
				break; 
			
			default:
			_puts("No valid code selected");
				continue;
			}
			_puts("\n-------\n\n");
			_delay_ms(1000);
	 }
}
