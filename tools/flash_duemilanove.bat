@echo off
break ON
rem fichiers BAT et fork créés par Sébastien CANET
SET currentpath=%~dp1
cd %currentpath%tools
cls
.\avrdude -Cavrdude.conf -v -patmega328p -carduino -P\\.\COM5 -b57600 -D -V -Uflash:w:s2aio-FirmataPlus.Duemilanove.hex:i
pause