@echo off
break ON
rem fichiers BAT et fork cr��s par S�bastien CANET
SET currentpath=%~dp1
cd %currentpath%tools
cls
.\avrdude -Cavrdude.conf -v -patmega2560 -cwiring -P\\.\COM8 -b115200 -D -V -Uflash:w:s2aio-FirmataPlus.Mega.hex:i
pause