// =======================================================================
// Proyecto VIDEOCONSOLA TETRIS Curso 2021-2022
// Fecha: 30/06/2022 
// Autor: Victoria Pacho Velasco y Andrés Martín Yeves
// Asignatura: Desarrollo Práctico de Sistemas Electrónicos
// File: mcp3004.c  Rutinas para manejar el ADC
// ======================================================================= 
#include "system.h"
#include "MCP3004.h"   
        
#ifndef ADC_CS
#define ADC_CS (1<<9)
#endif
//ADC_CS = 10 0000 0000 
// -----------------------------------------------------------------
#define MCP3004_Select()    FIOCLR = ADC_CS;
#define MCP3004_Unselect()  FIOSET = ADC_CS;
// -----------------------------------------------------------------
#define CMD_START (0b00000001)
#define CMD_CH0   (0b10000000)
#define CMD_CH1   (0b10010000)
#define CMD_CH2   (0b10100000)
#define CMD_CH3   (0b10110000)
// ----------------------------------------------------------------
//SPI READ -> [pag 15,18 Datasheet MCP3004]

unsigned char MCP3004_EnviaRecibe(unsigned char d)
{
 // d: Dato que se quiere enviar
  S0SPDR = d;
  while(! (S0SPSR&(1<<7)) ); // Se queda hasta que termina la transf de datos
  
 return S0SPDR; //Se devuelve el dato transmitido o recibido
 
 // --- INFO DE LOS REGISTROS:
 //SPSR = SPI Status Register. This register shows the status of the SPI
   // 7 SPIF SPI transfer complete flag. When 1, this bit indicates when a SPI
   //data transfer is complete. (el bit 7) XOOO OOOO = 1
 //SPDR = SPI Data Register
   //provides the transmit and receive data for the SPI.
   //Transmit data is provided to the SPI by
   //writing to this register. Data received by the
   //SPI can be read from this register.
}
// -----------------------------------------------------------------
int MCP3004_Read(unsigned char cmd_ch)
{
	//SPI READ -> [pag 18 Datasheet MCP3004]
	MCP3004_Select(); //Iniciar comunicacion SPI. FIOCLR = ADC_CS;
		   
	MCP3004_EnviaRecibe(CMD_START);// Enviar el bit de START (BYTE CMD_START)
	unsigned char B_MSB, B_LSB; //1 Byte
	unsigned short B; //2 Bytes = (B_MSB B_LSB)
	B_MSB = MCP3004_EnviaRecibe(cmd_ch); // Envio los bits de seleccion de canal 
	//(BYTE CMD_CHi) [(diff)(D2)(D1)(D0) XXXX]
	//Cuidado! A la vez, recibo los primeros datos [???? ?0(B9)(B8)]
	B_LSB = MCP3004_EnviaRecibe(0); // Dummy Byte [0000 0000]
	//Cuidado! A la vez, recibo los segundos datos [(B7)(B6)(B5)(B4) (B3)(B2)(B1)(B0)]
	MCP3004_Unselect(); //Finalizar comunicacion SPI. FIOSET = ADC_CS;
	
	B = B_MSB<<8; // Desplazo el dato 8 bits a la izquierda (B_MSB) (0000 0000)
	B|=B_LSB; // XOR con B_LSB -> (B_MSB) (0000 0000) XOR (0000 0000) (B_LSB)
	// Añadir mascara de 10 bits (0x3ff) 0b0000001111111111
	return (B&=0b0000001111111111);
}
// -----------------------------------------------------------------
void MCP3004_Init()
{

 FIODIR |= ADC_CS; // Almacena en FIODIR = (FIODIR orBitABit ADC_CS)
 
 // FIODIR Fast GPIO Port Direction control register.
   //This register individually controls the direction of each port pin.
}
// -----------------------------------------------------------------
void MCP3004_PrintCH(int channel, int *s)
{
	// Elige y guarda el valor del canal que quiero sacar en *s.
	// Al llamar MCP3004_PrintCH(2, &b)
	// Después de la llamada tengo en b el contenido del canal 2
	 switch ( channel )
	{
		case 0:
			// Joystick X
			*s=MCP3004_Read(CMD_CH0);
			break;
		case 1:
			// Joystick Y
			*s=MCP3004_Read(CMD_CH1);
			break;
		case 2:
			// Bateria
			*s=MCP3004_Read(CMD_CH2);
			break;
		default:
			break;
	}
}


// =======================================================================
// Proyecto VIDEOCONSOLA TETRIS Curso 2021-2022
// Fecha: 30/06/2022 
// Autor: Victoria Pacho Velasco y Andrés Martín Yeves
// Asignatura: Desarrollo Práctico de Sistemas Electrónicos
// File: mcp3004.h  Definiciones para manejar el ADC
// =======================================================================
 
#ifndef MCP3004_h
#define MCP3004_h

void MCP3004_Init();
unsigned char MCP3004_EnviaRecibe(unsigned char d);
int MCP3004_Read(unsigned char cmd_ch);
void MCP3004_PrintCH(int channel, int *s);

#endif
 //MCP3004
	// CMD_CH0 = (0b10000000) -> Joy X
	// CMD_CH1 = (0b10010000) -> Joy Y
	// CMD_CH2 = (0b10100000) -> BAT
	// CMD_CH3 = (0b10110000) -> ---