// =======================================================================
// Proyecto Datalogger for IoT Curso 2022-2023
// Fecha:25/01/2022 
// Autor: Pablo Villacorta, Rubén Serrano, Óscar Martín y Andrés Martín
// Asignatura: Taller de Proyectos I
// File: test.c  
// =======================================================================

void test(){
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
	TCNT=CCLK; //Configuramos el reloj cada segundo 
	
	     
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
			_puts("-> q: Salta a la direccion 0 (casi como un reset)\n");			
			_puts("-> t: Prueba el temporizador de los LED (T = 0.5 seg, al arrancar 1 seg) [BLOQ]\n");
			_puts("-> g: Decodificar Trama GPS (Espera trama)\n");
			_puts("-> h: La salida del GPS (UART1) a la UART0 [BLOQ]\n");
			_puts("-> k: La salida de la UART2 a la UART0 [BLOQ]\n");
			_puts("-> l: Lee estado del TIMER\n");
			_puts("-> 1: Pinta menu por UART0\n");
			_puts("-> 2: Envia datos por UART0 via interrupciones \n");
			_puts("-> 3: Lectura GPIN \n"); 
			_puts("-> G: Activar sensor GAS \n");
			_puts("-> P: Activar sensor Polvo \n");
			_puts("-> L: Transmitir datos por LoRA \n");
			_puts("-> T: Ver Gas/Polvo \n\n");
			
			_puts("Command [z456789rqtgkl123GPLT]> ");
			char cmd = _getch();
			if (cmd > 32 && cmd < 127)				
				_putch(cmd);
				_puts("\n");
 
			switch (cmd){
				case 'z': //Lee los registros del transceptor LoRa
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
					//IRQEN |= IRQEN_U2RX;
					while(1){
					_putch(UART2DAT); //No contiene nada
					}
					break;	

				case 'l': //Lee estado del TIMER 
					_printf("\nTCNT: %d\n",TCNT);
					break;	

				case '1': //Pinta menú por UART0
					_puts(menutxt);
					break;
  
				case '2': //Envía datos por UART0 vía interrupciones
					IRQEN=IRQEN_U0TX; 
					while(1){
					UART0DAT='A';
					}
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
				 
				case 'L': //Transmitir datos por LoRa 
					transmitPRAOFrame();			

					break;  

				case 'T': //Activar Sensor Polvo
					printCO(); 
					printDust();   
					printCh4LPG();
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