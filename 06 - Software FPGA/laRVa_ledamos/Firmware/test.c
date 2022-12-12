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

// ---- PRUEBA TEMPORIZADOR ----
void timer(void)
{

}