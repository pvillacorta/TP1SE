// =======================================================================
// Proyecto Datalogger for IoT Curso 2022-2023
// Fecha:07/12/2022 
// Autor: Pablo Villacorta, Rubén Serrano, Óscar Martín y Andrés Martín
// Asignatura: Taller de Proyectos I
// File: bme680.c  Rutinas para controlar el Sensor BME680 
// =======================================================================

// BME Registers
#define status_BME  		(*(volatile uint8_t*)0x73) 	//Reset
#define reset_BME  			(*(volatile uint8_t*)0x60)
#define Id_BME  			(*(volatile uint8_t*)0x50)
#define Config_BME  		(*(volatile uint8_t*)0x75)
#define Ctrl_meas_BME  		(*(volatile uint8_t*)0x74)  // osrs_h
#define Ctrl_hum_BME  		(*(volatile uint8_t*)0x72)
#define Ctrl_gas_1_BME  	(*(volatile uint8_t*)0x71)
#define Ctrl_gas_0_BME  	(*(volatile uint8_t*)0x70)
#define press_msb_BME  		(*(volatile uint8_t*)0x1f)

#define RD	(0b10000000)
#define WR	(0b00000000)	

// ---- PRUEBA DE DEPURACION INTERRUPCIONES ----
// --------------------------------------------------------

uint8_t spixfer (uint8_t d)
{
	SPIDAT=d;
	while(SPISTA&1);
	return SPIDAT;
}

// --------------------------------------------------------

void readAllRegs(void) // 
{
	char dataread=5;
	_puts("Registro press_msb_BME antes:\n");
	SPISS=BME680_CS;
	spixfer(press_msb_BME|RD); // rw = 0 write, rw=1 read
	dataread = spixfer(0x00);	//send dummy byte
	_printf("Valor del reg: %h\n",dataread);
	_delay_ms(100);
}