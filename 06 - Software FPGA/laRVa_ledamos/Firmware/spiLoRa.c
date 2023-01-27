// =======================================================================
// Proyecto Datalogger for IoT Curso 2022-2023
// Fecha: 26/01/2023 
// Autor: Pablo Villacorta, Rubén Serrano, Óscar Martín y Andrés Martín
// Asignatura: Taller de Proyectos I
// File: spiLoRa.c  Rutinas para controlar el Sensor LoRA 
// =======================================================================

/////////////////////
// LoRA Registers  //
/////////////////////

// This is the maximum number of interrupts the driver can support
// Most Arduinos can handle 2, Megas can handle more
#define RH_RF95_NUM_INTERRUPTS 3

// Max number of octets the LORA Rx/Tx FIFO can hold
#define RH_RF95_FIFO_SIZE 255

// This is the maximum number of bytes that can be carried by the LORA.
// We use some for headers, keeping fewer for RadioHead messages
#define RH_RF95_MAX_PAYLOAD_LEN RH_RF95_FIFO_SIZE

// The length of the headers we add.
// The headers are inside the LORA's payload
#define RH_RF95_HEADER_LEN 4

// This is the maximum message length that can be supported by this driver. 
// Can be pre-defined to a smaller size (to save SRAM) prior to including this header
// Here we allow for 1 byte message length, 4 bytes headers, user data and 2 bytes of FCS
#ifndef RH_RF95_MAX_MESSAGE_LEN
 #define RH_RF95_MAX_MESSAGE_LEN (RH_RF95_MAX_PAYLOAD_LEN - RH_RF95_HEADER_LEN)
#endif

// The crystal oscillator frequency of the module
#define RH_RF95_FXOSC 32000000.0

// The Frequency Synthesizer step = RH_RF95_FXOSC / 2^^19
#define RH_RF95_FSTEP  (RH_RF95_FXOSC / 524288)

#define RH_RF95_REG_00_FIFO                                0x00
#define RH_RF95_REG_01_OP_MODE                             0x01
#define RH_RF95_REG_02_RESERVED                            0x02
#define RH_RF95_REG_03_RESERVED                            0x03
#define RH_RF95_REG_04_RESERVED                            0x04
#define RH_RF95_REG_05_RESERVED                            0x05
#define RH_RF95_REG_06_FRF_MSB                             0x06
#define RH_RF95_REG_07_FRF_MID                             0x07
#define RH_RF95_REG_08_FRF_LSB                             0x08
#define RH_RF95_REG_09_PA_CONFIG                           0x09
#define RH_RF95_REG_0A_PA_RAMP                             0x0a
#define RH_RF95_REG_0B_OCP                                 0x0b
#define RH_RF95_REG_0C_LNA                                 0x0c
#define RH_RF95_REG_0D_FIFO_ADDR_PTR                       0x0d
#define RH_RF95_REG_0E_FIFO_TX_BASE_ADDR                   0x0e
#define RH_RF95_REG_0F_FIFO_RX_BASE_ADDR                   0x0f
#define RH_RF95_REG_10_FIFO_RX_CURRENT_ADDR                0x10
#define RH_RF95_REG_11_IRQ_FLAGS_MASK                      0x11
#define RH_RF95_REG_12_IRQ_FLAGS                           0x12
#define RH_RF95_REG_13_RX_NB_BYTES                         0x13
#define RH_RF95_REG_14_RX_HEADER_CNT_VALUE_MSB             0x14
#define RH_RF95_REG_15_RX_HEADER_CNT_VALUE_LSB             0x15
#define RH_RF95_REG_16_RX_PACKET_CNT_VALUE_MSB             0x16
#define RH_RF95_REG_17_RX_PACKET_CNT_VALUE_LSB             0x17
#define RH_RF95_REG_18_MODEM_STAT                          0x18
#define RH_RF95_REG_19_PKT_SNR_VALUE                       0x19
#define RH_RF95_REG_1A_PKT_RSSI_VALUE                      0x1a
#define RH_RF95_REG_1B_RSSI_VALUE                          0x1b
#define RH_RF95_REG_1C_HOP_CHANNEL                         0x1c
#define RH_RF95_REG_1D_MODEM_CONFIG1                       0x1d
#define RH_RF95_REG_1E_MODEM_CONFIG2                       0x1e
#define RH_RF95_REG_1F_SYMB_TIMEOUT_LSB                    0x1f
#define RH_RF95_REG_20_PREAMBLE_MSB                        0x20
#define RH_RF95_REG_21_PREAMBLE_LSB                        0x21
#define RH_RF95_REG_22_PAYLOAD_LENGTH                      0x22
#define RH_RF95_REG_23_MAX_PAYLOAD_LENGTH                  0x23
#define RH_RF95_REG_24_HOP_PERIOD                          0x24
#define RH_RF95_REG_25_FIFO_RX_BYTE_ADDR                   0x25
#define RH_RF95_REG_26_MODEM_CONFIG3                       0x26

#define RH_RF95_REG_27_PPM_CORRECTION                      0x27
#define RH_RF95_REG_28_FEI_MSB                             0x28
#define RH_RF95_REG_29_FEI_MID                             0x29
#define RH_RF95_REG_2A_FEI_LSB                             0x2a
#define RH_RF95_REG_2C_RSSI_WIDEBAND                       0x2c
#define RH_RF95_REG_31_DETECT_OPTIMIZE                     0x31
#define RH_RF95_REG_33_INVERT_IQ                           0x33
#define RH_RF95_REG_37_DETECTION_THRESHOLD                 0x37
#define RH_RF95_REG_39_SYNC_WORD                           0x39

#define RH_RF95_REG_40_DIO_MAPPING1                        0x40
#define RH_RF95_REG_41_DIO_MAPPING2                        0x41
#define RH_RF95_REG_42_VERSION                             0x42

#define RH_RF95_REG_4B_TCXO                                0x4b
#define RH_RF95_REG_4D_PA_DAC                              0x4d
#define RH_RF95_REG_5B_FORMER_TEMP                         0x5b
#define RH_RF95_REG_61_AGC_REF                             0x61
#define RH_RF95_REG_62_AGC_THRESH1                         0x62
#define RH_RF95_REG_63_AGC_THRESH2                         0x63
#define RH_RF95_REG_64_AGC_THRESH3                         0x64

// RH_RF95_REG_01_OP_MODE                             0x01
#define RH_RF95_LONG_RANGE_MODE                       0x80
#define RH_RF95_ACCESS_SHARED_REG                     0x40
#define RH_RF95_LOW_FREQUENCY_MODE                    0x08
#define RH_RF95_MODE                                  0x07
#define RH_RF95_MODE_SLEEP                            0x00
#define RH_RF95_MODE_STDBY                            0x01
#define RH_RF95_MODE_FSTX                             0x02
#define RH_RF95_MODE_TX                               0x03
#define RH_RF95_MODE_FSRX                             0x04
#define RH_RF95_MODE_RXCONTINUOUS                     0x05
#define RH_RF95_MODE_RXSINGLE                         0x06
#define RH_RF95_MODE_CAD                              0x07

// RH_RF95_REG_09_PA_CONFIG                           0x09
#define RH_RF95_PA_SELECT                             0x80
#define RH_RF95_MAX_POWER                             0x70
#define RH_RF95_OUTPUT_POWER                          0x0f

// RH_RF95_REG_0A_PA_RAMP                             0x0a
#define RH_RF95_LOW_PN_TX_PLL_OFF                     0x10
#define RH_RF95_PA_RAMP                               0x0f
#define RH_RF95_PA_RAMP_3_4MS                         0x00
#define RH_RF95_PA_RAMP_2MS                           0x01
#define RH_RF95_PA_RAMP_1MS                           0x02
#define RH_RF95_PA_RAMP_500US                         0x03
#define RH_RF95_PA_RAMP_250US                         0x04
#define RH_RF95_PA_RAMP_125US                         0x05
#define RH_RF95_PA_RAMP_100US                         0x06
#define RH_RF95_PA_RAMP_62US                          0x07
#define RH_RF95_PA_RAMP_50US                          0x08
#define RH_RF95_PA_RAMP_40US                          0x09
#define RH_RF95_PA_RAMP_31US                          0x0a
#define RH_RF95_PA_RAMP_25US                          0x0b
#define RH_RF95_PA_RAMP_20US                          0x0c
#define RH_RF95_PA_RAMP_15US                          0x0d
#define RH_RF95_PA_RAMP_12US                          0x0e
#define RH_RF95_PA_RAMP_10US                          0x0f

// RH_RF95_REG_0B_OCP                                 0x0b
#define RH_RF95_OCP_ON                                0x20
#define RH_RF95_OCP_TRIM                              0x1f

// RH_RF95_REG_0C_LNA                                 0x0c
#define RH_RF95_LNA_GAIN                              0xe0
#define RH_RF95_LNA_GAIN_G1                           0x20
#define RH_RF95_LNA_GAIN_G2                           0x40
#define RH_RF95_LNA_GAIN_G3                           0x60                
#define RH_RF95_LNA_GAIN_G4                           0x80
#define RH_RF95_LNA_GAIN_G5                           0xa0
#define RH_RF95_LNA_GAIN_G6                           0xc0
#define RH_RF95_LNA_BOOST_LF                          0x18
#define RH_RF95_LNA_BOOST_LF_DEFAULT                  0x00
#define RH_RF95_LNA_BOOST_HF                          0x03
#define RH_RF95_LNA_BOOST_HF_DEFAULT                  0x00
#define RH_RF95_LNA_BOOST_HF_150PC                    0x03

// RH_RF95_REG_11_IRQ_FLAGS_MASK                      0x11
#define RH_RF95_RX_TIMEOUT_MASK                       0x80
#define RH_RF95_RX_DONE_MASK                          0x40
#define RH_RF95_PAYLOAD_CRC_ERROR_MASK                0x20
#define RH_RF95_VALID_HEADER_MASK                     0x10
#define RH_RF95_TX_DONE_MASK                          0x08
#define RH_RF95_CAD_DONE_MASK                         0x04
#define RH_RF95_FHSS_CHANGE_CHANNEL_MASK              0x02
#define RH_RF95_CAD_DETECTED_MASK                     0x01

// RH_RF95_REG_12_IRQ_FLAGS                           0x12
#define RH_RF95_RX_TIMEOUT                            0x80
#define RH_RF95_RX_DONE                               0x40
#define RH_RF95_PAYLOAD_CRC_ERROR                     0x20
#define RH_RF95_VALID_HEADER                          0x10
#define RH_RF95_TX_DONE                               0x08
#define RH_RF95_CAD_DONE                              0x04
#define RH_RF95_FHSS_CHANGE_CHANNEL                   0x02
#define RH_RF95_CAD_DETECTED                          0x01

// RH_RF95_REG_18_MODEM_STAT                          0x18
#define RH_RF95_RX_CODING_RATE                        0xe0
#define RH_RF95_MODEM_STATUS_CLEAR                    0x10
#define RH_RF95_MODEM_STATUS_HEADER_INFO_VALID        0x08
#define RH_RF95_MODEM_STATUS_RX_ONGOING               0x04
#define RH_RF95_MODEM_STATUS_SIGNAL_SYNCHRONIZED      0x02
#define RH_RF95_MODEM_STATUS_SIGNAL_DETECTED          0x01

// RH_RF95_REG_1C_HOP_CHANNEL                         0x1c
#define RH_RF95_PLL_TIMEOUT                           0x80
#define RH_RF95_RX_PAYLOAD_CRC_IS_ON                  0x40
#define RH_RF95_FHSS_PRESENT_CHANNEL                  0x3f

// RH_RF95_REG_1D_MODEM_CONFIG1                       0x1d
#define RH_RF95_BW                                    0xf0

#define RH_RF95_BW_7_8KHZ                             0x00
#define RH_RF95_BW_10_4KHZ                            0x10
#define RH_RF95_BW_15_6KHZ                            0x20
#define RH_RF95_BW_20_8KHZ                            0x30
#define RH_RF95_BW_31_25KHZ                           0x40
#define RH_RF95_BW_41_7KHZ                            0x50
#define RH_RF95_BW_62_5KHZ                            0x60
#define RH_RF95_BW_125KHZ                             0x70
#define RH_RF95_BW_250KHZ                             0x80
#define RH_RF95_BW_500KHZ                             0x90
#define RH_RF95_CODING_RATE                           0x0e
#define RH_RF95_CODING_RATE_4_5                       0x02
#define RH_RF95_CODING_RATE_4_6                       0x04
#define RH_RF95_CODING_RATE_4_7                       0x06
#define RH_RF95_CODING_RATE_4_8                       0x08
#define RH_RF95_IMPLICIT_HEADER_MODE_ON               0x01

// RH_RF95_REG_1E_MODEM_CONFIG2                       0x1e
#define RH_RF95_SPREADING_FACTOR                      0xf0
#define RH_RF95_SPREADING_FACTOR_64CPS                0x60
#define RH_RF95_SPREADING_FACTOR_128CPS               0x70
#define RH_RF95_SPREADING_FACTOR_256CPS               0x80
#define RH_RF95_SPREADING_FACTOR_512CPS               0x90
#define RH_RF95_SPREADING_FACTOR_1024CPS              0xa0
#define RH_RF95_SPREADING_FACTOR_2048CPS              0xb0
#define RH_RF95_SPREADING_FACTOR_4096CPS              0xc0
#define RH_RF95_TX_CONTINUOUS_MODE                    0x08

#define RH_RF95_PAYLOAD_CRC_ON                        0x04
#define RH_RF95_SYM_TIMEOUT_MSB                       0x03

// RH_RF95_REG_26_MODEM_CONFIG3
#define RH_RF95_MOBILE_NODE                           0x08 // HopeRF term
#define RH_RF95_LOW_DATA_RATE_OPTIMIZE                0x08 // Semtechs term
#define RH_RF95_AGC_AUTO_ON                           0x04

// RH_RF95_REG_4B_TCXO                                0x4b
#define RH_RF95_TCXO_TCXO_INPUT_ON                    0x10

// RH_RF95_REG_4D_PA_DAC                              0x4d
#define RH_RF95_PA_DAC_DISABLE                        0x04
#define RH_RF95_PA_DAC_ENABLE                         0x07

/////////////////////
/////////////////////

#define RD2	(0b10000000) // rw = 1 write, rw=0 read	

/////////////////////
//  AÑADIDO PRAO   //
/////////////////////

#define RHModeSleep			0x00
#define RHModeIdle			0x01
#define RHModeTx			0x02
#define RHModeRx			0x03

uint8_t _mode;

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

void printLoRaRegs() 
{ 
	int i,j;

	_puts("Registros del modulo LoRa:\n");

	for(i=0;i<8;i++){
		for(j=0;j<16;j++){
			_printf("%02x ",readLoRA(0x00 + 16*i + j));
		}
		_puts("\n");
	}
}



void setModemRegisters(){
	writeLoRA(0x72, RH_RF95_REG_1D_MODEM_CONFIG1);
	writeLoRA(0x74, RH_RF95_REG_1E_MODEM_CONFIG2);
	writeLoRA(0x04, RH_RF95_REG_26_MODEM_CONFIG3);
}

// Los anteriores registros son de onfiguración
// El primero es de BW
// El segundo es de Cr (para lo del codigo de lora)
// El ultimo es de Sf (para lo del codigo de lora)
//  1d,     1e,      26
// {0x72,   0xb4,    0x04}, // Bw125Cr45Sf2048, AGC enabled
// {0x72,   0x74,    0x04}, // Bw125Cr45Sf128 (the chip default), AGC enabled
// {0x92,   0x74,    0x04}, // Bw500Cr45Sf128, AGC enabled
// {0x48,   0x94,    0x04}, // Bw31_25Cr48Sf512, AGC enabled
// {0x78,   0xc4,    0x0c}, // Bw125Cr48Sf4096, AGC enabled

void setPreambleLength(uint16_t bytes){
	writeLoRA(bytes >> 8, RH_RF95_REG_20_PREAMBLE_MSB);
	writeLoRA(bytes & 0xff, RH_RF95_REG_21_PREAMBLE_LSB);
}

void setFrequency868(){
	writeLoRA(0xD9, RH_RF95_REG_06_FRF_MSB);
	writeLoRA(0x00, RH_RF95_REG_07_FRF_MID);
	writeLoRA(0x00, RH_RF95_REG_08_FRF_LSB);
}

#define _txHeaderTo		 0xff
#define _txHeaderFrom	 0xff 
#define _txHeaderId 	 0x00
#define _txHeaderFlags	 0x00

void loraSend(char *mensaje){
	uint8_t size = _sizeof(mensaje);

	// STDBY Mode:
	writeLoRA(RH_RF95_MODE_STDBY | RH_RF95_LONG_RANGE_MODE, RH_RF95_REG_01_OP_MODE);

	writeLoRA(0, RH_RF95_REG_0D_FIFO_ADDR_PTR); // Puntero a la direccion inicial en la fifo

	writeLoRA(size+4,RH_RF95_REG_22_PAYLOAD_LENGTH); // Longitud de los datos
	
	// Write data FIFO ---------------------------------------------------

	// Cabeceras
	writeLoRA(_txHeaderTo, RH_RF95_REG_00_FIFO);
	writeLoRA(_txHeaderFrom, RH_RF95_REG_00_FIFO);
	writeLoRA(_txHeaderId, RH_RF95_REG_00_FIFO);
	writeLoRA(_txHeaderFlags, RH_RF95_REG_00_FIFO);

	// _puts("Reg IRQ antes de tx:\n");
	// _printfBin(readLoRA(RH_RF95_REG_12_IRQ_FLAGS));

	_puts("Transmitiendo");
	//el registro reg_fifo contiene el contenido de la fifo al que apunta la direccion fifo_addr_ptr
	//cada vez que escribimos/leemos en reg_fifo se incrementa el registro fifo_addr_ptr en 1
	for(uint8_t i = 0; i<size; i++){
		writeLoRA(*mensaje++,RH_RF95_REG_00_FIFO);
	}

	writeLoRA(RH_RF95_MODE_TX | RH_RF95_LONG_RANGE_MODE, RH_RF95_REG_01_OP_MODE); // Modo transmisión
	_delay_ms(1);
	
	while(1){
		if(readLoRA(RH_RF95_REG_01_OP_MODE) == (RH_RF95_MODE_TX | RH_RF95_LONG_RANGE_MODE)){
			_puts(".");
			_delay_ms(1);
		}
		 //Tras la transmisión, cambia automáticamente al modo STAND-BY:
		if(readLoRA(RH_RF95_REG_01_OP_MODE) == (RH_RF95_MODE_STDBY | RH_RF95_LONG_RANGE_MODE)){
			_puts("\nTransmision completada\n");
			writeLoRA(0xff,RH_RF95_REG_12_IRQ_FLAGS); // Bajamos el flag TxDone poniendo todo el registro irq_flags a 0xff
			break;
		}
	}

	// Vuelta del puntero a la dirección 0:
	writeLoRA(0, RH_RF95_REG_0D_FIFO_ADDR_PTR); // Puntero a la direccion inicial en la fifo
}

uint8_t loraInit(){ 
 	// Set sleep mode, so we can also set LORA mode:
	writeLoRA(RH_RF95_MODE_SLEEP | RH_RF95_LONG_RANGE_MODE, RH_RF95_REG_01_OP_MODE);

 	// Check if we are in sleep mode:
	if (readLoRA(RH_RF95_REG_01_OP_MODE) != (RH_RF95_MODE_SLEEP | RH_RF95_LONG_RANGE_MODE)){
		_puts("No se detecta LoRa\n");
		return 0; // No device present?
	}

	// Set up FIFO
    // We configure so that we can use the entire 256 byte FIFO for either receive
    // or transmit, but not both at the same time
    writeLoRA(0, RH_RF95_REG_0E_FIFO_TX_BASE_ADDR);
    writeLoRA(0, RH_RF95_REG_0F_FIFO_RX_BASE_ADDR);
	
	// STDBY Mode:
	writeLoRA(RH_RF95_MODE_STDBY | RH_RF95_LONG_RANGE_MODE, RH_RF95_REG_01_OP_MODE);
	
	setModemRegisters();
	setPreambleLength(8);
	setFrequency868(); 

	// Set TX Power -------
	writeLoRA(0xAD, RH_RF95_REG_09_PA_CONFIG);
	//---------------------
}


void transmitPRAOFrame(){
	char *strtemp;
	char *strhum;
	char *strpresion;
	char *strpolvo;
	char *bufferTX;
	char *strCO;
	char *strCh4LPG;
	uint8_t indx= 0;
	
	loraInit(); //Inicializa el módulo LoRa
	
	// 1a transmision:
	loraSend("\n$$ INIT PRAO TX $$"); //Transmite
	
	// 2a transmision: (UTC TIME)
	bufferTX="";
	_memcpy(_memcpy(bufferTX, "\n$ UTC Time: ",_sizeof("\n$ UTC Time: ")) - 1, UTC_time, _sizeof(UTC_time));
	loraSend(bufferTX); //Transmite
	_puts(bufferTX); 
	_puts("\n");
	
	// 3A transmision: (DATE)
	bufferTX="";
	_memcpy(_memcpy(bufferTX, "\n$ Date: ",_sizeof("\n$ Date: ")) - 1, date, _sizeof(date));
	loraSend(bufferTX); //Transmite
	_puts(bufferTX); 
	_puts("\n");
	
	// 3a transmision: (Temperatura)
	bufferTX="";
	my_itoa(temp,strtemp);
	_memcpy(_memcpy(_memcpy(bufferTX, "\n$ Temp: ",_sizeof("\n$ Temp: ")) - 1, strtemp, _sizeof(strtemp))- 1, "deg", _sizeof("deg"));
	loraSend(bufferTX); //Transmite
	_puts(bufferTX); 
	_puts("\n");
	
	// 4a transmision: (Humedad)
	bufferTX=""; // Limpio el buffer
	my_itoa(humedad,strhum);
	_memcpy(_memcpy(_memcpy(bufferTX, "\n$ Humedad: ",_sizeof("\n$ Humedad: ")) - 1, strhum, _sizeof(strhum))- 1, "%", _sizeof("%"));
	_puts(bufferTX);
	_puts("\n");
	loraSend(bufferTX); //Transmite
	
	// 5a transmision: (Presión)
	bufferTX=""; // Limpio el buffer
	my_itoa(presion,strpresion);
	_memcpy(_memcpy(_memcpy(bufferTX, "\n$ Presion: ",_sizeof("\n$ Presion: ")) - 1, strpresion, _sizeof(strpresion))- 1, "hPas", _sizeof("hPas"));
	_puts(bufferTX);
	_puts("\n");
	loraSend(bufferTX); //Transmite
	
	// 6a transmision: (Polvo)
	bufferTX=""; // Limpio el buffer
	my_itoa(polvoValue,strpolvo);
	_memcpy(_memcpy(_memcpy(bufferTX, "\n$ Polvo: ",_sizeof("\n$ Polvo: ")) - 1, strpolvo, _sizeof(strpolvo))- 1, "mV ADC", _sizeof("mV ADC"));
	_puts(bufferTX);
	_puts("\n");
	loraSend(bufferTX); //Transmite
	
	// 7a transmision: (CO)
	bufferTX=""; // Limpio el buffer
	my_itoa(COValue,strCO);
	_memcpy(_memcpy(_memcpy(bufferTX, "\n$ CO: ",_sizeof("\n$ CO: ")) - 1, strCO, _sizeof(strCO))- 1, "mV ADC", _sizeof("mV ADC"));
	_puts(bufferTX);
	_puts("\n");
	loraSend(bufferTX); //Transmite
	
	// 8a transmision: (CH4)
	bufferTX=""; // Limpio el buffer
	my_itoa(Ch4LPGValue,strCh4LPG);
	_memcpy(_memcpy(_memcpy(bufferTX, "\n$ Ch4LPG: ",_sizeof("\n$ Ch4LPG: ")) - 1, strCh4LPG, _sizeof(strCh4LPG))- 1, "mV ADC", _sizeof("mV ADC"));
	_puts(bufferTX);
	_puts("\n");
	loraSend(bufferTX); //Transmite
	
	//Ultima transmision:
	loraSend("\n$$ END PRAO TX $$"); //Transmite
}

