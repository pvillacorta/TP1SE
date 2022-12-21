// =======================================================================
// Proyecto Datalogger for IoT Curso 2022-2023
// Fecha:07/12/2022 
// Autor: Pablo Villacorta, Rubén Serrano, Óscar Martín y Andrés Martín
// Asignatura: Taller de Proyectos I
// File: test.c  Rutinas de Test del GPS
// =======================================================================

// ---- PRUEBA DE DEPURACION INTERRUPCIONES ----
//La hemos utilizamos al principio del main para
//depuración del temporizador

void test_depurFirst(void) // 
{
	UART0BAUD=(CCLK+BAUD0/2)/BAUD0 -1;	
	_puts("U");
	IRQVECT3=(uint32_t)irq3_handler; //Timer
	IRQEN=IRQEN_TIMER;
	TCNT=CCLK; //CCLK = 18000000 -> Equivale a 1seg
}

 /////////////////////////////
// UART
// ---- PRUEBA DE LECTURA DESDE LA NUEVA UART1 ----
void test_U1_IRQREAD(void) // Y lo escribe por UART0
{
		char uart1_data = _getch1();
		_putch(uart1_data);
}
// ------------------------------------------------
void test_U1_READ(void) // Y lo escribe por UART0
{	 
	while((UART1STA&1)==0); // Comprueba el flag DV (Si esta a 0 se queda esperando al dato)
	_putch(UART1DAT);

	//uint8_t _getch1() // LEER DE UART1 DIRECTAMENTE
	//{
	//	while((UART1STA&1)==0); // Comprueba el flag DV (Si esta a 0 se queda esperando al dato)
	//	return UART1DAT;
	//}
}

// ---- PRUEBA DE ESCRITURA DESDE LA UART0 ----
void test_U0_WRITE(uint8_t caracter)
{
	_putch(caracter);
}
 /////////////////////////////
// TEMPORIZADOR
// ---- PRUEBA TEMPORIZADOR ----
void timerTest(void)
{	
	IRQEN=IRQEN_TIMER; // Habilitar Interrupcion del timer
	TCNT=CCLK; //CCLK = 18000000 -> Equivale a 1seg

}
 /////////////////////////////
// GPOUT

// ---- PRUEBA GPOUT ----
void GpoutTest(void)
{
	_delay_ms(1000);
	GPOUT= 0;
	_delay_ms(1000);
	GPOUT= ice_led1;
	_delay_ms(1000);
	GPOUT= ice_led2;
	_delay_ms(1000);
	GPOUT= ice_led3;
	_delay_ms(1000);
	GPOUT= ice_led4;
	 
}

 /////////////////////////////
// GPIN

// ---- PRUEBA GPIN ----
void GpinTest(void)
{
	char Gpin_test;
	Gpin_test=GPIN;
	_putch('\n');
	
	if (Gpin_test&0b10000000)
		_putch('1');
	else
		_putch('0');
	if (Gpin_test&0b01000000)
		_putch('1');
	else
		_putch('0');
	if (Gpin_test&0b00100000)
		_putch('1');
	else
		_putch('0');
	if (Gpin_test&0b00010000)
		_putch('1');
	else
		_putch('0');
	if (Gpin_test&0b00001000)
		_putch('1');
	else
		_putch('0');
	if (Gpin_test&0b00000100)
		_putch('1');
	else
		_putch('0');
	if (Gpin_test&0b00000010)
		_putch('1');
	else
		_putch('0');
	if (Gpin_test&0b00000001)
		_putch('1');
	else
		_putch('0');
}

 /////////////////////////////
// GPIN

void SPITest(void)
{
	// Escritura en el Slave Select
	SPICTL = (8<<8)|8;  
	SPISS=BME680_CS;
	
	while(1){
		spixfer('A');
		_delay_ms(0.01);
	}
	
	
}