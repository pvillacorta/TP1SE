// =======================================================================
// Proyecto Datalogger for IoT Curso 2022-2023
// Fecha:17/01/2023 
// Autor: Pablo Villacorta, Rubén Serrano, Óscar Martín y Andrés Martín
// Asignatura: Taller de Proyectos I
// File: LoRA.c  Rutinas para controlar el Sensor LoRA 
// =======================================================================

/////////////////////
// LoRA Registers  //
/////////////////////

#define RD2	(0b10000000) // rw = 0 write, rw=1 read	

// ---- PRUEBA DE DEPURACION INTERRUPCIONES ----

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
	spiLoRAxfer (RD2|dir);
	dataread = spiLoRAxfer(0x00);
	SPILSS=1;
	return dataread; //send dummy byte
}

// --------------------------------------------------------

void writeLoRA(char data,char dir){
	SPILSS=0;
	spiLoRAxfer (dir);
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