# Anotaciones de Datalogger for IoT (Taller de Proyectos 1)

## Programación FPGA 🛠️
Para comilar y cargar programas en la FPGA se debe hacer lo siguiente:

1. `make` dentro de la carpeta 'Firmware' para cargar el programa a ejecutar en la memoria
2. `make burn` dentro de la carpeta 'LaRVa'

## Diseño PCB ⚙️

* Planos de masa: `Tools > Power Plane Generator`
* Autorutado: `Design Rule Manager (Icono de escuadra y lápiz) > Autoruter` 
* Ocultar o visualizar etiquetas: `Edit Layer Colors/Visibility (Icono de 3 hojas)`, se puede voltear la PCB mediante `Toggle Board Flip`

## Diseño Esquemático 🔩
* Nombrar todos los componentes en orden: `Tools > Global Anotator > Total / Current Board`
* Para generar los ficheros de todas las placas PCB: `File > Print Design: page orientation: Landscape, All sheets, Print in Colour, Printer: Microsoft Print to PDF`

## Generación BOM 💵
* Para cambiar las unidades del dinero: `Unit Cost (Edit BOM Field) > Field Value > Prefix/Sufix`
* Para generar el documento correspondiente al Bill of Materials hacer click sobre el icono PDF

## Diagrama de GANT 📈
* Para generar el diagrama de manera correcta y completa visualización: `Proyecto > Exportar > Informe PDF > Fechas: 20-09 a 19-02`


## Solución Errores Git 💻
* Problemas con VIM (Repositorio de GitHub), salimos con la secuencia de teclas: _ESC, :q, Enter_


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



## Autores ✒️
* **Andrés Martín**
* **Óscar Martín**
* **Pablo Villacorta**
* **Rubén Serrano**
---
Estudiantes del Master de Ingeniero de Telecomunicación en la Universidad de Valladolid

