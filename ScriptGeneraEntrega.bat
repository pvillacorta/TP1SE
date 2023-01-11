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

IF EXIST "%ArchivoDestino%" (
    ECHO El archivo "%ArchivoDestino%" ya existe
) ELSE (md "%ArchivoDestino%")

IF EXIST "%ArchivoDestino%\00 - Doc" (
    ECHO El archivo "%ArchivoDestino%\00 - Doc" ya existe
) ELSE (md "%ArchivoDestino%\00 - Doc")

IF EXIST "%ArchivoDestino%\01 - Planification" (
    ECHO El archivo "%ArchivoDestino%\01 - Planification" ya existe
) ELSE (md "%ArchivoDestino%\01 - Planification")

IF EXIST "%ArchivoDestino%\02 - Design" (
    ECHO El archivo "%ArchivoDestino%\02 - Design" ya existe
) ELSE (md "%ArchivoDestino%\02 - Design")

IF EXIST "%ArchivoDestino%\03 - AnalisisConsumo" (
    ECHO El archivo "%ArchivoDestino%\03 - AnalisisConsumo" ya existe
) ELSE (md "%ArchivoDestino%\03 - AnalisisConsumo")

IF EXIST "%ArchivoDestino%\04 - Software" (
    ECHO El archivo "%ArchivoDestino%\04 - Software" ya existe
) ELSE (md "%ArchivoDestino%\04 - Software")

:: --- Doc ---
XCOPY "00 - Documentacion\Informe_proyecto.docx" "%ArchivoDestino%\00 - Doc" /S /Y /Q
IF /I "%ERRORLEVEL%" NEQ "0" (ECHO ERROR -  No se ha podido copiar el archivo "00 - Documentacion\Informe_proyecto.docx")

:: --- Planification ---
XCOPY "01 - Planificacion del Trabajo\ControlCambios.txt" "%ArchivoDestino%\01 - Planification" /S /Y /Q
IF /I "%ERRORLEVEL%" NEQ "0" (ECHO ERROR -  No se ha podido copiar el archivo "01 - Planificacion del Trabajo\ControlCambios.txt")
XCOPY "01 - Planificacion del Trabajo\DIagramaGannt_EnCurso.pdf" "%ArchivoDestino%\01 - Planification" /S /Y /Q
IF /I "%ERRORLEVEL%" NEQ "0" (ECHO ERROR -  No se ha podido copiar el archivo "01 - Planificacion del Trabajo\DIagramaGannt_EnCurso.pdf")
XCOPY "01 - Planificacion del Trabajo\DIagramaGannt_Inicial.pdf" "%ArchivoDestino%\01 - Planification" /S /Y /Q
IF /I "%ERRORLEVEL%" NEQ "0" (ECHO ERROR -  No se ha podido copiar el archivo "01 - Planificacion del Trabajo\DIagramaGannt_Inicial.pdf")


:: --- Design ---
::  (1)- Esquematico -
XCOPY "03 - Proyecto Proteus\Esquematico\Esquematico. Datalogger for IoT.prn" "%ArchivoDestino%\02 - Design" /S /Y /Q
IF /I "%ERRORLEVEL%" NEQ "0" (ECHO ERROR -  No se ha podido copiar el archivo "03 - Proyecto Proteus\Esquematico\Esquematico. Datalogger for IoT.prn")
::  (2)- Proyecto Proteus -
XCOPY "03 - Proyecto Proteus\Datalogger_for_IoT_PRAO.pdsprj" "%ArchivoDestino%\02 - Design" /S /Y /Q
IF /I "%ERRORLEVEL%" NEQ "0" (ECHO ERROR -  No se ha podido copiar el archivo "03 - Proyecto Proteus\Datalogger_for_IoT_PRAO.pdsprj")
::  (3)- BOM -
XCOPY "03 - Proyecto Proteus\BOM" "%ArchivoDestino%\02 - Design\BOM" /S /Y /I /Q
IF /I "%ERRORLEVEL%" NEQ "0" (ECHO ERROR -  No se ha podido copiar el archivo "03 - Proyecto Proteus\BOM")
::  (4)- Printed Layouts -
XCOPY "03 - Proyecto Proteus\Printed Layouts" "%ArchivoDestino%\02 - Design\Layouts" /S /Y /I /Q
IF /I "%ERRORLEVEL%" NEQ "0" (ECHO ERROR -  No se ha podido copiar el archivo "03 - Proyecto Proteus\Printed Layouts")

:: --- AnalisisConsumo ---
XCOPY "04 - Analisis consumo\Analisis Electrico y Termico PCB0.pdf" "%ArchivoDestino%\03 - AnalisisConsumo" /S /Y /Q
IF /I "%ERRORLEVEL%" NEQ "0" (ECHO ERROR -  No se ha podido copiar el archivo "04 - Analisis consumo\Analisis Electrico y Termico PCB0.pdf")
XCOPY "04 - Analisis consumo\Analisis Electrico y Termico PCB1.pdf" "%ArchivoDestino%\03 - AnalisisConsumo" /S /Y /Q
IF /I "%ERRORLEVEL%" NEQ "0" (ECHO ERROR -  No se ha podido copiar el archivo "04 - Analisis consumo\Analisis Electrico y Termico PCB1.pdf") 
XCOPY "04 - Analisis consumo\Analisis Electrico y Termico PCB2.pdf" "%ArchivoDestino%\03 - AnalisisConsumo" /S /Y /Q
IF /I "%ERRORLEVEL%" NEQ "0" (ECHO ERROR -  No se ha podido copiar el archivo "04 - Analisis consumo\Analisis Electrico y Termico PCB2.pdf")
XCOPY "04 - Analisis consumo\Analisis Electrico y Termico PCB3.pdf" "%ArchivoDestino%\03 - AnalisisConsumo" /S /Y /Q
IF /I "%ERRORLEVEL%" NEQ "0" (ECHO ERROR -  No se ha podido copiar el archivo "04 - Analisis consumo\Analisis Electrico y Termico PCB3.pdf")


:: --- Software ---
XCOPY "06 - Software FPGA" "%ArchivoDestino%\04 - Software" /S /Y /I /Q
IF /I "%ERRORLEVEL%" NEQ "0" (ECHO ERROR -  No se ha podido copiar el archivo "06 - Software FPGA")

echo ---------------------------------

echo Documentacion generada con exito! & ::Este es un comentario

@pause


:: DOCUMENTACIÓN: (/S /Y /I ...)
:: https://www.minitool.com/es/respaldar-datos/comando-xcopy.html