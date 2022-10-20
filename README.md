# Control de errores - Datalogger for IoT
 
Este grupo está formado por: 
  - Andrés
  - Pablo
  - Rubén
  - Óscar
  
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