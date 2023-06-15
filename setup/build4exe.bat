@echo Create setup...
@echo off
@copy ..\out\release\FlyScp.exe .\packages\cn.flysnow.flyscp\data\FlyScp.exe /Y
@d:\Qt\QtIFW-3.2.2\bin\binarycreator.exe -c .\config\config.xml -p .\packages .\release\FlyScp1.0-win-x64.exe
@echo Create setup OK
@echo Compress setup...
@copy /b .\config\7zS.sfx + .\config\config.txt .\release\7zS.sfx
@7z a .\release\FlyScp1.0-win-x64_%date:~0,4%%date:~5,2%%date:~8,2%.exe -sfx.\release\7zS.sfx .\release\FlyScp1.0-win-x64.exe
@echo Compress setup OK.
@del .\release\FlyScp1.0-win-x64.exe .\release\7zS.sfx
pause