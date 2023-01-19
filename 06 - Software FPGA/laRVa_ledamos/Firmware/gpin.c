// =======================================================================
// Proyecto Datalogger for IoT Curso 2022-2023
// Fecha:07/12/2022 
// Autor: Pablo Villacorta, Rubén Serrano, Óscar Martín y Andrés Martín
// Asignatura: Taller de Proyectos I
// File: gpin.c  
// =======================================================================

// GPIN 
#define GPIN0 		(0b00000001)	//GPIN0 
#define GPIN1 		(0b00000010)	//GPIN0 
#define GPIN2 		(0b00000100)	//GPIN0  
#define GPIN3 		(0b00001000)	//GPIN0  
#define GPIN4 		(0b00010000)	//GPIN0 
#define GPIN5 		(0b00100000)	//GPIN0 
#define GPIN6 		(0b01000000)	//GPIN0 
#define GPIN7 		(0b10000000)	//GPIN0

 // ---- Lectura GPIN ----
void GpinRead(void)
{
	uint8_t Gpin_test;
	Gpin_test=GPIN;
	_puts("GPIN:");
	_printfBin(Gpin_test);
	_puts("\n");
}