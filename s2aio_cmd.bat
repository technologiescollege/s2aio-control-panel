@echo off
break ON
rem fichiers BAT et fork cr��s par S�bastien CANET
SET currentpath=%~dp1
cd %currentpath%s2aio
cls
python.exe .\Lib\site-packages\s2aio\__main__.py -l 5 -c no_client -p COM1