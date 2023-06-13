@echo Create setup...
@echo off
copy ..\out\release\FlyScp.exe .\packages\cn.flysnow.flyscp\data\FlyScp.exe /Y
d:\Qt\QtIFW-3.2.2\bin\binarycreator.exe -c .\config\config.xml -p .\packages .\release\FlyScp1.0-win-x64_%date:~0,4%%date:~5,2%%date:~8,2%.exe
@echo Create setup OK
@echo Compress setup...
7z a .\release\FlyScp1.0-win-x64_%date:~0,4%%date:~5,2%%date:~8,2%.zip .\release\FlyScp1.0-win-x64_%date:~0,4%%date:~5,2%%date:~8,2%.exe
@echo Compress setup OK.
del .\release\FlyScp1.0-win-x64_%date:~0,4%%date:~5,2%%date:~8,2%.exe
pause