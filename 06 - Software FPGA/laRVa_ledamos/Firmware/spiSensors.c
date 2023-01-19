// =======================================================================
// Proyecto Datalogger for IoT Curso 2022-2023
// Fecha:17/01/2022 
// Autor: Pablo Villacorta, Rubén Serrano, Óscar Martín y Andrés Martín
// Asignatura: Taller de Proyectos I
// File: spiSensors.c  
//
// Rutinas para controlar el módulo SPI0. Este SPI se encarga de la comunicación
// con el BME680 y con el ADC (a su vez, al ADC entran las salidas de los sensores
// analógicos)
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



void printBMERegs(){
	int i,j;
	
	_puts("Registros del sensor BME:\n");
	
	for(i=8;i<16;i++){
		for(j=0;j<16;j++){
			_printf("%02x ",bmeRegs[i][j]);
		}
		_puts("\n");
	}
	
	_puts("\n");
	
	for(i=0;i<8;i++){
		for(j=0;j<16;j++){
			_printf("%02x ",bmeRegs[i][j]);
		}
		_puts("\n");
	}
}



char bmeReg(char dir){
	int row, col;
	
	row = dir/16;
	col = dir%16;
	
	return bmeRegs[row][col];
}


// -------------- BME init ---------------------
void startBME680(){ 
	// Página 1
	writeBME680(0b00010000,status_BME);
	
	// Set humidity oversampling to 1x (osrs_h = Ctrl_hum_BME[2:0] = 001)
	writeBME680(0b00000001,Ctrl_hum_BME);
	_delay_ms(1);
	
	// Set temperature oversampling to 2x (osrs_t = Ctrl_meas_BME[7:5] = 010)
	// Set pressure oversampling to 16x (osrs_p = Ctrl_meas_BME[4:2] = 101)
	writeBME680(0b01010100,Ctrl_meas_BME);
	_delay_ms(1);
	
	// Set nb_conv to 0x0 to select the previously defined heater settings (nb_conv = Ctrl_gas_1_BME[3:0])
	// Set run_gas_l to 1 to enable gas measurements  (run_gas_l = Ctrl_gas_1_BME[4])
	writeBME680(0b00010000,Ctrl_gas_1_BME);
	_delay_ms(1);
	
	// Set gas_wait_0 to 0x59 to select 100 ms heat up duration
	writeBME680(0x59,gas_wait0);
	_delay_ms(1);
	
	// Set the corresponding heater set-point by writing the target heater resistance to res_heat_0  
	writeBME680(0,res_heat0);
	_delay_ms(1);
	
	// Set mode to 0b01 to trigger a single measurement. (mode = Ctrl_meas_BME[1:0])
	writeBME680(0b01010101,Ctrl_meas_BME);
	_delay_ms(1);
}



void measureBME680(){
// Read all BME registers
readAllBMERegs();
	
/*---------------------- Temperature measurement -----------------------------
	LSB / MSB
	par_t1 0xE9 / 0xEA  
	par_t2 0x8A / 0x8B  
	par_t3 0x8C  
	temp_adc 0x24<7:4>/ 0x23 / 0x22 
*/

	int par_t1, par_t2, par_t3, temp_adc;
	int var1, var2, var3, t_fine, temp_comp;
	
	par_t1 = (int32_t)((bmeReg(0xEA) << 8) | bmeReg(0xE9));
	par_t2 = (int32_t)((bmeReg(0x8B) << 8) | bmeReg(0x8A));
	par_t3 = (int32_t)bmeReg(0x8C);
	temp_adc = (int32_t)((bmeReg(0x22)  << 12) | (bmeReg(0x23) << 4) | (bmeReg(0x24) >> 4)); 
	
	var1 = ((int32_t)temp_adc >> 3) - ((int32_t)par_t1 << 1);
	var2 = (var1 * (int32_t)par_t2) >> 11;
	var3 = ((((var1 >> 1) * (var1 >> 1)) >> 12) * ((int32_t)par_t3 << 4)) >> 14;
	t_fine = var2 + var3;
	temp_comp = ((t_fine * 5) + 128) >> 8;


/* ------------------------ Pressure measurement ----------------------------------- 

	LSB / MSB
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

	int par_p1, par_p2, par_p3, par_p4, par_p5, par_p6, par_p7, par_p8, par_p9, par_p10, press_adc;
	int press_comp;

	par_p1 = (int32_t)((bmeReg(0x8F) << 8) | bmeReg(0x8E));
	par_p2 = (int32_t)((bmeReg(0x91) << 8) | bmeReg(0x90));
	par_p3 = (int32_t)bmeReg(0x92);
	par_p4 = (int32_t)((bmeReg(0x95) << 8) | bmeReg(0x94));
	par_p5 = (int32_t)((bmeReg(0x97) << 8) | bmeReg(0x96));
	par_p6 = (int32_t)bmeReg(0x99);
	par_p7 = (int32_t)bmeReg(0x98);
	par_p8 = (int32_t)((bmeReg(0x9D) << 8) | bmeReg(0x9C));
	par_p9 = (int32_t)((bmeReg(0x9F) << 8) | bmeReg(0x9E));
	par_p10 = (int32_t)bmeReg(0xA0);
	press_adc = (int32_t)((bmeReg(0x1F) << 12) | (bmeReg(0x20) << 4) | (bmeReg(0x21) >> 4));

	var1 = ((int32_t)t_fine >> 1) - 64000;
	var2 = ((((var1 >> 2) * (var1 >> 2)) >> 11) * (int32_t)par_p6) >> 2;
	var2 = var2 + ((var1 * (int32_t)par_p5) << 1);
	var2 = (var2 >> 2) + ((int32_t)par_p4 << 16);
	var1 = (((((var1 >> 2) * (var1 >> 2)) >> 13) * ((int32_t)par_p3 << 5)) >> 3) + (((int32_t)par_p2 * var1) >> 1);
	var1 = var1 >> 18;
	var1 = ((32768 + var1) * (int32_t)par_p1) >> 15;
	press_comp = 1048576 - press_adc;
	press_comp = (uint32_t)((press_comp - (var2 >> 12)) * ((uint32_t)3125));
	if (press_comp >= (1 << 30)) press_comp = ((press_comp / (uint32_t)var1) << 1);
	else press_comp = ((press_comp << 1) / (uint32_t)var1);
	var1 = ((int32_t)par_p9 * (int32_t)(((press_comp >> 3) * (press_comp >> 3)) >> 13)) >> 12;
	var2 = ((int32_t)(press_comp >> 2) * (int32_t)par_p8) >> 13;
	var3 = ((int32_t)(press_comp >> 8) * (int32_t)(press_comp >> 8) * (int32_t)(press_comp >> 8) * (int32_t)par_p10) >> 17;
	press_comp = (int32_t)(press_comp) + ((var1 + var2 + var3 + ((int32_t)par_p7 << 7)) >> 4);
	
	
/* ----------------------------- Humidity measurement ------------------------------------
	
	LSB / MSB
	par_h1 <3:0>0xE2 / 0xE3  
	par_h2 <7:4>0xE2 / 0xE1  
	par_h3 0xE4  
	par_h4 0xE5  
	par_h5 0xE6  
	par_h6 0xE7  
	par_h7 0xE8  
	hum_adc 0x26 / 0x25   
*/

	int par_h1, par_h2, par_h3, par_h4, par_h5, par_h6, par_h7, hum_adc;
	int temp_scaled, hum_comp, var4, var5, var6;
	
	par_h1 = (int32_t)((bmeReg(0xE3) << 4) | (bmeReg(0xE2) & 0b00001111));
	par_h2 = (int32_t)((bmeReg(0xE1) << 4) | (bmeReg(0xE2) >> 4));
	par_h3 = (int32_t)bmeReg(0xE4);
	par_h4 = (int32_t)bmeReg(0xE5);
	par_h5 = (int32_t)bmeReg(0xE6);
	par_h3 = (int32_t)bmeReg(0xE7);
	par_h3 = (int32_t)bmeReg(0xE8);
	hum_adc = (int32_t)((bmeReg(0x25) << 8) | bmeReg(0x26));
	
	temp_scaled = (int32_t)temp_comp;
	var1 = (int32_t)hum_adc - (int32_t)((int32_t)par_h1 << 4) - (((temp_scaled * (int32_t)par_h3) / ((int32_t)100)) >> 1);
	var2 = ((int32_t)par_h2 * (((temp_scaled * (int32_t)par_h4) / ((int32_t)100)) + (((temp_scaled * ((temp_scaled * (int32_t)par_h5) / ((int32_t)100))) >> 6) / ((int32_t)100)) + ((int32_t)(1 << 14)))) >> 10;
	var3 = var1 * var2;
	var4 = (((int32_t)par_h6 << 7) + ((temp_scaled * (int32_t)par_h7) / ((int32_t)100))) >> 4;
	var5 = ((var3 >> 14) * (var3 >> 14)) >> 10;
	var6 = (var4 * var5) >> 1;
	hum_comp = (((var3 + var6) >> 10) * ((int32_t) 1000)) >> 12;
	
	// -----------------------------------------------------------------
	
	_printf("Temperatura: %d%cC\n", temp_comp/100, 167);
	_printf("temp_adc: %dn", temp_adc);
	_printf("Presion: %d hPascal\n", press_comp/100);
	_printf("Humedad: %d%c\n", hum_comp/1000, 37);
}



///////////////////////////
//         ADC           //
///////////////////////////

/*

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
	// A�adir mascara de 10 bits (0x3ff) 0b0000001111111111
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
	// Despu�s de la llamada tengo en b el contenido del canal 2
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

*/