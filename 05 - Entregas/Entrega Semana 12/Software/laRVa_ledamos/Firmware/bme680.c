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

#define gas_wait0			0x6D
#define gas_wait1			0x6C
#define gas_wait2			0x6B
#define gas_wait3			0x6A
#define gas_wait4			0x69
#define gas_wait5			0x68
#define gas_wait6			0x67
#define gas_wait7			0x66
#define gas_wait8			0x65
#define gas_wait9			0x64

#define res_heat0			0x63
#define res_heat1			0x62
#define res_heat2			0x61
#define res_heat3			0x60
#define res_heat4			0x5F
#define res_heat5			0x5E
#define res_heat6			0x5D
#define res_heat7			0x5C
#define res_heat8			0x5B
#define res_heat9			0x5A

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


// --------------------------------------------------------

uint8_t spixfer(uint8_t d)
{
	SPIDAT=d;
	while(SPISTA&1);
	return SPIDAT;
}

// --------------------------------------------------------

char readBME680(char dir){
	char dataread=0x00;
	SPISS=BME680_CS;
	spixfer(RD|dir);
	dataread = spixfer(0x00); //send dummy byte
	SPISS=0b11;
	return dataread; 
}

// --------------------------------------------------------

void writeBME680(char data,char dir){
	SPISS=BME680_CS;
	spixfer(dir);
	spixfer(data);
	SPISS=0b11;
}

/* --------------------- LECTURA DE TODOS LOS REGISTROS DEL BME -----------------------
	2 páginas de registros, se selecciona una página u otra con el registro spi_mem_page:
		spi_mem_page = 0: página 0 --> 128 registros (0x80 to 0xFF)
		spi_mem_page = 1: página 1 --> 128 registros (0x00 to 0x7F)
	128*2 = 256 registros
*/

char bmeRegs[16][16];

void readAllBMERegs() 
{ 
	int i,j;
	
	// Página 0
	writeBME680(0b00000000,status_BME);
	for(i=0;i<8;i++){
		for(j=0;j<16;j++){
			bmeRegs[i+8][j] = readBME680(0x80 + 16*i + j); 
		}
	}
	
	// Página 1
	writeBME680(0b00010000,status_BME);
	for(i=0;i<8;i++){
		for(j=0;j<16;j++){
			bmeRegs[i][j] = readBME680(0x00 + 16*i + j); 
		}
	}
}


// -------------- BME init ---------------------
void startBME680(){ 
	
	// Set humidity oversampling to 1x (osrs_h = Ctrl_hum_BME[2:0] = 001)
	writeBME680(0b00000001,Ctrl_hum_BME);
	
	// Set temperature oversampling to 2x (osrs_t = Ctrl_meas_BME[7:5] = 010)
	// Set pressure oversampling to 16x (osrs_p = Ctrl_meas_BME[4:2] = 101)
	writeBME680(0b01010100,Ctrl_meas_BME);
	
	// Set gas_wait_0 to 0x59 to select 100 ms heat up duration
	writeBME680(0x59,gas_wait0);
	
	// Set the corresponding heater set-point by writing the target heater resistance to res_heat_0  
	writeBME680(0,res_heat0);
	
	// Set nb_conv to 0x0 to select the previously defined heater settings (nb_conv = Ctrl_gas_1_BME[3:0])
	// Set run_gas_l to 1 to enable gas measurements  (run_gas_l = Ctrl_gas_1_BME[4])
	writeBME680(0b00010000,Ctrl_gas_1_BME);
	
	// Set mode to 0b01 to trigger a single measurement. (mode = Ctrl_meas_BME[1:0])
	writeBME680(0b01010101,Ctrl_meas_BME);
	
	// Read all BME registers
	readAllBMERegs();
	
	// Print all registers
	printBMERegs();
}


void printBMERegs(){
	int i,j;
	
	_puts("Registros del sensor BME:\n");
	
	for(i=0;i<16;i++){
		for(j=0;j<16;j++){
			_printf("0x%02x  ",bmeRegs[i][j]);
		}
		_puts("\n");
		if(i == 7){
			_puts("\n");
		}
	}
}



char bmeReg(char dir){
	int row, col;
	
	row = dir/16;
	col = dir%16;
	
	return bmeRegs[row][col];
}


/*---------------------- Temperature measurement -----------------------------

var1 = ((int32_t)temp_adc >> 3) - ((int32_t)par_t1 << 1);  
var2 = (var1 * (int32_t)par_t2) >> 11;  
var3 = ((((var1 >> 1) * (var1 >> 1)) >> 12) * ((int32_t)par_t3 << 4)) >> 14;  
t_fine = var2 + var3;  
temp_comp = ((t_fine * 5) + 128) >> 8; 

where  
par_t1 0xE9 / 0xEA  
par_t2 0x8A / 0x8B  
par_t3 0x8C  
temp_adc 0x24<7:4>/ 0x23 / 0x22 
*/

int tempBME680(){
	int par_t1, par_t2, par_t3, temp_adc;
	int var1, var2, var3, t_fine, temp_comp;
	
	par_t1 = (int32_t)((bmeReg(0xE9) << 8) | bmeReg(0xEA));
	par_t2 = (int32_t)((bmeReg(0x8A) << 8) | bmeReg(0x8B));
	par_t3 = (int32_t)bmeReg(0x8C);
	temp_adc = (int32_t)(((bmeReg(0x24) >> 4) << 16) | (bmeReg(0x23) << 8) | bmeReg(0x22)); 
	
	var1 = ((int32_t)temp_adc >> 3) - ((int32_t)par_t1 << 1);
	var2 = (var1 * (int32_t)par_t2) >> 11;
	var3 = ((((var1 >> 1) * (var1 >>1)) >> 12) * ((int32_t)par_t3 << 4)) >> 14;
	t_fine = var2 + var3;
	temp_comp = ((t_fine * 5) + 128) >> 8;
	
	return temp_comp;
}


/* ------------------------ Pressure measurement ----------------------------------- 
 
var1 = ((int32_t)t_fine >> 1) - 64000;  
var2 = ((((var1 >> 2) * (var1 >> 2)) >> 11) * (int32_t)par_p6) >> 2;  
var2 = var2 + ((var1 * (int32_t)par_p5) << 1);  
var2 = (var2 >> 2) + ((int32_t)par_p4 << 16);  
var1 = (((((var1 >> 2) * (var1 >> 2)) >> 13) * ((int32_t)par_p3 << 5)) >> 3) + (((int32_t)par_p2 * var1) >> 1);  
var1 = var1 >> 18;  
var1 = ((32768 + var1) * (int32_t)par_p1) >> 15;  
press_comp = 1048576 - press_raw;  
press_comp = (uint32_t)((press_comp - (var2 >> 12)) * ((uint32_t)3125));  
if (press_comp >= (1 << 30)) press_comp = ((press_comp / (uint32_t)var1) << 1);  
else press_comp = ((press_comp << 1) / (uint32_t)var1);  
var1 = ((int32_t)par_p9 * (int32_t)(((press_comp >> 3) * (press_comp >> 3)) >> 13)) >> 12;  
var2 = ((int32_t)(press_comp >> 2) * (int32_t)par_p8) >> 13;  
var3 = ((int32_t)(press_comp >> 8) * (int32_t)(press_comp >> 8) * (int32_t)(press_comp >> 8) * (int32_t)par_p10) >> 17;  
press_comp = (int32_t)(press_comp) + ((var1 + var2 + var3 + ((int32_t)par_p7 << 7)) >> 4); 

where 
par_p1 0x8E / 0x8F  
par_p2 0x90 / 0x91  
par_p3 0x92  
par_p4 0x94 / 0x95  
par_p5 0x96 / 0x97  
par_p6 0x99  
par_p7 0x98  
par_p8 0x9C / 0x9D  
par_p9 0x9E / 0x9F  
par_p10 0xA0  
press_adc 0x21<7:4> / 0x20 / 0x1F 
*/

int pressBME680(){	
	return 0;
}
