# Control de errores - Datalogger for IoT
 
Este grupo está formado por: 
  - Andrés
  - Pablo
  - Rubén
  - Óscar


---- 12/12/2022 ----- (A,O,P)
Fichero code.lst:
En este fichero podemos ver el código ensamblador y la dirección de memoria de cada una de sus instrucciones.
Nos puede servir para, por ejemplo, ver en qué dirección de memoria se guarda una rutina de atención a interrupción.

Sobre las simulaciones: es posible simular directamente el código c. De hecho, cuando se hace make (o make burn), primero se ejecuta el código ensamblador (start.S) y después el C (main.c). Por tanto, para que dé tiempo a que se ejecuten ambos códigos, basta con aumentar el tiempo de simulación. También se puede reducir el periodo del reloj, para aumentar el número total de ciclos de reloj simulados.

Sobre NotePad: podemos fijar puntos (breakpoints) y saltar entre ellos con F2(siguiente) y Shift+F2(anterior)

---- 07/12/2022 -- (A)
Observar fichero prn: Recoge todos los warning y errores tras ejecutar make burn
Por ejemplo se puede leer:
"ERROR: IO 'RXD0' is unconstrained in PCF (override this error with --pcf-allow-unconstrained)"
Esto es un error que se debe a que el pin no esta en el archivo pines.pcf

Además, en el fichero prn podemos visualizar la frecuencia máxima a la que podría funcionar el circuito:
Info: Max frequency for clock 'clk_$glb_clk': 21.79 MHz (PASS at 12.00 MHz)

COMPILADOR DE C ONLINE:
https://www.onlinegdb.com/online_c_compiler

-> NMEA ENCODER / DECODER:
https://simba-os.readthedocs.io/en/latest/library-reference/encode/nmea.html


---- 05/12/2022 -- (R,A) 
página web que permite hacer los scripts de verilog, te va diciendo si tienes algun error 
(como por ejemplo puntos y comas, fallos como meter un assign dentro de un always y esas cosas)
y además te va sacando las salidas mientras escibes (aunque a veces funciona regular)

https://8bitworkshop.com/

---- 29/11/2022 -- (A,R,P) 
pines.pcf (LarVa_ledamos) -> Fichero que define las equivalencias número pin
main.v (LarVa_ledamos) -> Añadir las entradas de la FPGA correspondientes al nuevo modulo que agregamos
En la instancia a system añadir nuevas entradas/salidas (por ej RXD2 y TXD2)
system.V (LarVa_ledamos) -> Añadir las entradas correspondientes a la instancia [Como hicimos con Ruth]

main.c (Firmware) -> Añadir rutinas, siempre definir posición de los registros arriba!


---- 03/11/2022 -- (A) 
Compilar y cargar programas en la FPGA:
- make burn en /larva_lesdoy

---- 03/11/2022 -- (A) 
Nombrar los componentes en orden
TOOL -> GLOBAL ANOTATOR -> TOTAL / CURRENT BOARD

---- 03/11/2022 -- (A) 
Sobre planificación de posicionado de componentes en PCB2:
El módulo sensor de polvo es enorme (lo mejor va a ser ponerlo fuera de la pcb)
Problema: Falta encontrar el transistor MMBT3903 ¿?

---- 29/10/2022 -- (A) 
CONTROL DE ESQUEMÁTICOS - HUELLAS SENSORES:
[MISO MOSI (Prot SPI) iguales para ADC y modulo sensor temperatura¿? creo que si porque ya lo seleccionas luego con ss1 o ss2 etc]
[I2C para el sensor pequeñin ¿?-> Supongo que hay que elegir los pines en la FPGA]
[Ojo con los agujeros en las huellas!]
- Regulador conmutado síncrono XC9141B50DMR-G: Creado (Huella creada pines corresponden, revisar medidas) [Que pasa con el CE¿?]
- ADC: Creado (Faltan pines ocultos NC 5 y 6) Huella SO14 de DPSE comprobada. [/ss2 se refiere a adc_cs¿?]
- TEL0132 Módulo Receptor de GPS: [Comprobar huella]
- SEN0134 Módulo sensor de gases: únicamente 3 pines, Las medidas parece que pueden ser buenas
- BME680 Module (Modulo de Sensores temp...): Esquemático correcto para interfaz SPI. Las medidas son correctas [Hay que modelar los huecos de la huella para los taladros?]
- MS860702BA01-50 (Sensor de Temperatura pequeño): Demasiado pequeño, lo vamos a montar hacia arriba dado la vuelta (estan las dos huellas para elegir)
- GP2Y1010AU0F (Sensor de polvo): le falta modelar bien la superficie en la huella (el agujero esta) [El conector S6B-ZR-SM4A-TF del sensor de polvo, lo agregamos al esquema aunque no va en pcb? Exclude from pcb]

[Los canales que utilizamos del ADC son: 1 para el sensor de gases, otro para el de polvo y otro para medir el nivel de consumo de la batería?]




---- 20/10/2022 -- (A)
Nota sobre las unidades del precio en el BOM:
- Ir a las propiedades de "Unit Cost" (Edit BOM Field) > Field Value > Prefix/Sufix

Problema con la orientación de las etiquetas en pcb0?¿ -> Ver Layout Bottom (En el 3D Visualizer parece OK)

Faltan Planos de masa en PCB1

Falta Diseño 3D en PCB1: L77HDE15SD1CH4F [Depuración reunión Jesús]
-------

---- 11/10/2022 -- (A)
Nota sobre los planos de masa:
(En el menú de PCB) Tools -> Power Plane Generator

Nota sobre el autorutado:
(Icono de escuadra y lapiz) -> Design rule manager
Luego: (Icono de cables cruzados al lado) -> autoruter

Nota acerca del PCB:
Podemos ocultar o visualizar las etiquetas o elementos mediante la herramienta Edita Layer Colors/Visibility (Icono tres hojas, amarilla, azul, roja)
Para darle la vuelta se pulsar Toggle Board Flip

Nota problemas con el vim (repositorio):
Para salir, Secuencia de teclas: ESC, :q, ENTER
-------

---- 30/09/2022 -- (A)
Nota sobre la generación del documento asociado al diagrama de Gannt:
Proyecto > Exportar > Informe PDF > Fechas: 20-09 a 19-02 se ve correctamente

Nota sobre la generación del documento correspondiente al esquemático:
File > Print Design: page orientation: Landscape, All sheets, Print in Colour, Printer: Microsoft Print to PDF 

Nota sobre la generación del documento correspondiente al Bill of Materials:
Hacer click en el icono PDF y lo saca directamente

!!SOLVED!!
Error con el package del cristal de cuarzo
Package 'XTAL_12000MHZB2T' not found for component 'X1'.
-------