@echo off
break ON
rem fichiers BAT et fork cr��s par S�bastien CANET
SET currentpath=%~dp1
cd %currentpath%tools
cls
.\avrdude -Cavrdude.conf -v -patmega32u4 -cavr109 -P\\.\COM8 -b57600 -D -V -Uflash:w:s2a-FirmataPlus.Micro.hex:i
pause