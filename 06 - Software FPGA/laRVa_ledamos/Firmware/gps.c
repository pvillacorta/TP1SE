// =======================================================================
// Proyecto Datalogger for IoT Curso 2022-2023
// Fecha: 17/01/2022 
// Autor: Pablo Villacorta, Rub�n Serrano, �scar Mart�n y Andr�s Mart�n
// Asignatura: Taller de Proyectos I
// File: gps.c  Programa controlador del GPS
// =======================================================================

// El GPS se encuentra conectado a trav�s de la UART1

//FIFO UART 1:
//uint8_t udat1[128]; //FIFO de recepcion para la UART1 (tama�o 1028 bits)

//volatile uint8_t rdixU1,wriU2; // Punteros de lectura y escritura (unsigned char -> 8 bit)

// -----------------------------------------------------------------------


// ==============================================================================
// ------------------------        FUNCIONES         ----------------------------
// ==============================================================================


char * strchr( const char str[], char ch ) 
{
    while ( *str && *str != ch ) ++str;

    return ( char * )( ch == *str ? str : "\0" );  
}

int parse_comma_delimited_str(char *string, char **fields, int max_fields)
{
	int i = 0;
	fields[i++] = string;
	//while ((i < max_fields) && strchr(string,',') != NULL) {
	while ((i < max_fields) && "\0" != (string = strchr(string, ','))) {
		*string = '\0';
		fields[i++] = ++string; 
	}

	return --i;
}

int strncmp(char *string1, char *buscar){  //esta medio general, se puede generalizar mas, pero pereza
	uint8_t contador=0;
	uint8_t retorno=0;
	while(string1[contador+1]== buscar[contador]){
		retorno++;
		contador++; // HASTA AQUI LO HACE BIEN, COMPROBADO
		if(retorno == 6){
				return 0;
		}
	}
	return -1;
}

uint8_t getGPSFrame() //LEE DEL GPS un Frame Completo
{
	int i;
	char *field[20];
	uint8_t comprobador=0;
	volatile uint8_t pointer=0;
	_puts("\nWaiting for conexion ($GNGGA,$GPGGA,$GNRMC & $GPRMC) \n");
	//while (comprobador <=1){ //Espera dos
	while (comprobador <=0){ //Solo coje una	
		while(GPS_FF == '0'){
			GPS_FRAME[pointer]=_getch1();
			pointer++;
		}
		GPS_FF = '0';
		if ((strncmp(GPS_FRAME,"$GNGGA") == 0)||(strncmp(GPS_FRAME,"$GPGGA") == 0)){ //comparamos cadena con las que queremos encontrar

				parse_comma_delimited_str(GPS_FRAME, field, 20);
				
				_puts("---------------INFORMACION GPS---------------\n");
				_puts("UTC Time  :\t");_puts("hora: ");_putch(field[1][0]);_putch(field[1][1]);
				_puts("  minutos: ");_putch(field[1][2]);_putch(field[1][3]);
				_puts("  segundos: ");_putch(field[1][4]);_putch(field[1][5]);_putch('\n');
				_puts("Latitude  :\t");_puts(field[2]);_putch('\n');		
				_puts("Longitude :\t");_puts(field[4]);_putch('\n');
				_puts("Altitude  :\t");_puts(field[9]);_putch('\n');
				_puts("Satellites:\t");_puts(field[7]);_putch('\n');
				
				UTC_time[0] = field[1][0];
				UTC_time[1] = field[1][1];
				UTC_time[2] = ':';
				UTC_time[3] = field[1][2];
				UTC_time[4] = field[1][3];
				UTC_time[5] = ':';
				UTC_time[6] = field[1][4];
				UTC_time[7] = field[1][5];
				UTC_time[8]='\0';
				

				//_memcpy(latitude, field[2],10);
				//_memcpy(altitude, field[9],10);
				//_memcpy(longitude, field[4],10);
				//_memcpy(satelites, field[7],10);

				
				
				comprobador++;	
		}
		if ((strncmp(GPS_FRAME,"$GNRMC") == 0)||(strncmp(GPS_FRAME,"$GPRMC") == 0)){ //comparamos cadena con las que queremos encontrar
				parse_comma_delimited_str(GPS_FRAME, field, 20);
				
				_puts("Date   :\t");_puts("dia: ");_putch(field[9][0]);_putch(field[9][1]);
				_puts("  mes: ");_putch(field[9][2]);_putch(field[9][3]);
				_puts("  anio: ");_putch(field[9][4]);_putch(field[9][5]);_putch('\n');
				_putch('\n');_putch('\n');
				
				date[0]=field[9][0];
				date[1]=field[9][1];
				date[2]='-';
				date[3]=field[9][2];
				date[4]=field[9][3];
				date[5]='-';
				date[6]=field[9][4];
				date[7]=field[9][5];
				date[8]='\0';
				
				
				comprobador++;
		}
		for(i=0;i<pointer-1;i++){ 
			GPS_FRAME[i]=0;			//para limpiar la cadena
		}
	}
	return pointer;
}

