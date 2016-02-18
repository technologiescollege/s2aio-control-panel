@echo off
break ON
rem fichiers BAT créés par Sébastien CANET
SET currentpath=%~dp0
SET arduinopath=S:\Perso\GitHub\technologies\arduino\
cd %arduinopath%
S:
arduino.exe %currentpath%s2aio\blocklytemp\blocklytemp.ino