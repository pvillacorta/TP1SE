// =======================================================================
// Proyecto Datalogger for IoT Curso 2022-2023
// Fecha:17/01/2023 
// Autor: Pablo Villacorta, Rubén Serrano, Óscar Martín y Andrés Martín
// Asignatura: Taller de Proyectos I
// File: spiLoRa.c  Rutinas para controlar el Sensor LoRA 
// =======================================================================



/////////////////////
// LoRA Registers  //
/////////////////////
#include <spiLora.h>

#define RD2	(0b10000000) // rw = 1 write, rw=0 read	

// --------- PRUEBA DE DEPURACION INTERRUPCIONES ----------

// --------------------------------------------------------

uint8_t spiLoRAxfer (uint8_t d)
{
	SPILDAT=d;
	while(SPILSTA&1);
	return SPILDAT;
}

// --------------------------------------------------------

char readLoRA(char dir){
	char dataread=0x00;
	SPILSS=0;
	spiLoRAxfer (dir);
	dataread = spiLoRAxfer(0x00);
	SPILSS=1;
	return dataread; //send dummy byte
}

// --------------------------------------------------------

void writeLoRA(char data,char dir){
	SPILSS=0;
	spiLoRAxfer (RD2|dir);
	spiLoRAxfer(data);
	SPILSS=1;
}


char loraRegs[8][16];

void readAllLoRaRegs() 
{ 
	int i,j;

	for(i=0;i<8;i++){
		for(j=0;j<16;j++){
			loraRegs[i][j] = readLoRA(0x00 + 16*i + j); 
		}
	}
}

void printLoRaRegs(){
	int i,j;
	
	_puts("Registros del modulo LoRa:\n");
	
	for(i=0;i<8;i++){
		for(j=0;j<16;j++){
			_printf("%02x ",loraRegs[i][j]);
		}
		_puts("\n");
	}
}

void loraInit(){
	// Set sleep mode, so we can also set LORA mode:
	writeLoRA(RH_RF95_MODE_SLEEP | RH_RF95_LONG_RANGE_MODE, RH_RF95_REG_01_OP_MODE);
	_delay_ms(10);

	// Check if we are in sleep mode:
	if (readLoRA(RH_RF95_REG_01_OP_MODE) != (RH_RF95_MODE_SLEEP | RH_RF95_LONG_RANGE_MODE)){
		return false; // No device present?
	}

	// Interrupciones ---------
	// ......
	// ------------------------


	// Set up FIFO
    // We configure so that we can use the entire 256 byte FIFO for either receive
    // or transmit, but not both at the same time
    writeLoRA(0, RH_RF95_REG_0E_FIFO_TX_BASE_ADDR);
    writeLoRA(0, RH_RF95_REG_0F_FIFO_RX_BASE_ADDR);
}