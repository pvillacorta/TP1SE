// =======================================================================
// Proyecto Datalogger for IoT Curso 2022-2023
// Fecha:07/12/2022 
// Autor: Pablo Villacorta, Rubén Serrano, Óscar Martín y Andrés Martín
// Asignatura: Taller de Proyectos I
// File: bme680.c  Rutinas para controlar el Sensor BME680 
// =======================================================================

////////////////////
// BME Registers  //
////////////////////

// READ / WRITE REGS
#define status_BME  		0x73   //Status [page]
#define reset_BME  			0x60
#define Id_BME  			0x50
#define Config_BME  		0x75
#define Ctrl_meas_BME  		0x74  // osrs_t [7:5] osrs_p [4:2] mode [1:0]
#define Ctrl_hum_BME  		0x72  // osrs_h [2:0]
#define Ctrl_gas_1_BME  	0x71
#define Ctrl_gas_0_BME  	0x70

// READ ONLY REGS
#define gas_r_lsb_BME		0x2B
#define gas_r_msb_BME		0x2A

#define hum_lsb_BME			0x26
#define hum_msb_BME			0x25

#define temp_xlsb_BME		0x24
#define temp_lsb_BME		0x23
#define temp_msb_BME		0x22

#define press_xlsb_BME  	0x21
#define press_lsb_BME  		0x20
#define press_msb_BME  		0x1F

#define eas_status_0_BME  	0x1D


#define RD	(0b10000000) // rw = 0 write, rw=1 read	


// ---- PRUEBA DE DEPURACION INTERRUPCIONES ----

// --------------------------------------------------------

uint8_t spixfer (uint8_t d)
{
	SPIDAT=d;
	while(SPISTA&1);
	return SPIDAT;
}

// --------------------------------------------------------

char readBME680(char dir){
	char dataread=0x00;
	SPISS=BME680_CS;
	spixfer (RD|dir);
	dataread = spixfer(0x00);
	SPISS=0b11;
	return dataread; //send dummy byte
}

// --------------------------------------------------------

void writeBME680(char data,char dir){
	SPISS=BME680_CS;
	spixfer (dir);
	spixfer(data);
	SPISS=0b11;
}

void readAllRegs(void) 
{ 
	char dataread;
	
	dataread = readBME680(Id_BME);
	_printf("Id_BME = ");
	_printfBin(dataread);
	
	dataread = readBME680(status_BME);
	_printf("status_BME = ");
	_printfBin(dataread);
	
	
	writeBME680(0b00000,status_BME);
	
	dataread = readBME680(status_BME);
	_printf("status_BME = ");
	_printfBin(dataread);
	
	
	//dataread = spixfer(0x00);	//send dummy byte
}