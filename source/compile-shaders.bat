@echo off

set PLATFORM = windows
set API = s_5_0

echo compile shaders

echo simple shader
..\mrender\tools\shaderc.exe ^
-f simple\simple-vert.sc -o simple\simple-vert.bin ^
--platform %PLATFORM% --type vertex --verbose -i ./ -p %API%

..\mrender\tools\shaderc.exe ^
-f simple\simple-frag.sc -o simple\simple-frag.bin ^
--platform %PLATFORM% --type fragment --verbose -i ./ -p %API%

PAUSE