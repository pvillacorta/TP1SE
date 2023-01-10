:: =======================================================================
:: Proyecto Datalogger for IoT Curso 2022-2023
:: Fecha: 10/01/2023 
:: Autor: Pablo Villacorta, Rubén Serrano, Óscar Martín y Andrés Martín
:: Asignatura: Taller de Proyectos I
:: File: scriptCopia.bat  (Script por lotes)
:: =======================================================================
@echo off
set "MM=%dt:~4,2%"
set "DD=%dt:~6,2%"
for /f "tokens=2 delims==" %%a in ('wmic OS Get localdatetime /value') do set "dt=%%a"
set "AA=%dt:~2,2%"
set "AAAA=%dt:~0,4%"
set "MM=%dt:~4,2%"
set "DD=%dt:~6,2%"
set "HH=%dt:~8,2%"
set "Min=%dt:~10,2%"

:: --- Creacion de Carpetas ---
set "dia_hora=%DD%_%MM%_%AA%"
set "ArchivoDestino=05 - Entregas\Entrega_%dia_hora%"
md "%ArchivoDestino%"
md "%ArchivoDestino%\00 - Doc"
md "%ArchivoDestino%\01 - Planification"
md "%ArchivoDestino%\02 - Design"
md "%ArchivoDestino%\03 - Software"

:: --- Proyecto Proteus ---
XCOPY "03 - Proyecto Proteus\Datalogger_for_IoT_PRAO.pdsprj" "%ArchivoDestino%\02 - Design" /S /Y

:: --- BOM ---
XCOPY "03 - Proyecto Proteus\BOM" "%ArchivoDestino%\02 - Design\BOM" /S /Y /I


:: --- Software ---
XCOPY "06 - Software FPGA" "%ArchivoDestino%\03 - Software" /S /Y /I

echo ---------------------------------
echo Documentacion generada con exito! & ::Este es un comentario

@pause


:: DOCUMENTACIÓN: (/S /Y /I ...)
:: https://www.minitool.com/es/respaldar-datos/comando-xcopy.html