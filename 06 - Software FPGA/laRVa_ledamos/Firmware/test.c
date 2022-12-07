// =======================================================================
// Proyecto Datalogger for IoT Curso 2022-2023
// Fecha:07/12/2022 
// Autor: Pablo Villacorta, Rubén Serrano, Óscar Martín y Andrés Martín
// Asignatura: Taller de Proyectos I
// File: test.c  Rutinas de Test del GPS
// =======================================================================


// ---- PRUEBA DE LECTURA DESDE LA NUEVA UART1 ----
void testUART1READ(void) // Y lo escribe por UART0
{
	while(1){
		char uart1_data = _getch1();
		_putch(uart1_data);
	}
}
// ------------------------------------------------

// ---- PRUEBA DE ESCRITURA DESDE LA UART0 ----
void testUART0WRITE(uint8_t caracter)
{
	while(1){
	_putch(caracter);
	}
}