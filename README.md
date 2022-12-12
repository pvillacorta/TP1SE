# Anotaciones de Datalogger for IoT [Taller de Proyectos 1]



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
* Para deshacer los commits hechos y volver al original: `git reset --hard HEAD~1`



## Autores ✒️
* **Andrés Martín**
* **Óscar Martín**
* **Pablo Villacorta**
* **Rubén Serrano**
---
Estudiantes del Master de Ingeniero de Telecomunicación en la Universidad de Valladolid

