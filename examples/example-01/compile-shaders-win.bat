@echo off

echo Compiling shaders ...

REM Variables
set SHADERS=debug_draw deferred_combine deferred_geo deferred_light_point deferred_light_directional deferred_light_spot screen shadow

REM Enter the current script directory in case we call this script from the application
cd /d "%~dp0"
echo Executing script from: %~dp0 ...

REM Loop through the list of shaders and compile them
for %%i in (%SHADERS%) do (
  call :compileVertexShader %%i
  call :compileFragmentShader %%i
)

REM Exit the loop and display the pause message
echo All shaders compiled successfully
PAUSE
EXIT /B %ERRORLEVEL%

REM Function to compile a vertex shader
:compileVertexShader
  echo Compiling vertex shader %1 ...
  build\bin\shaderc.exe ^
  -f shaders\src\%1\%1-vert.sc -o build\bin\data\shaders\%1-vert.bin ^
  --platform windows --profile 130 --type vertex --verbose -i ./ 
goto :eof

REM Function to compile a fragment shader
:compileFragmentShader
  echo Compiling fragment shader %1 ...
  build\bin\shaderc.exe ^
  -f shaders\src\%1\%1-frag.sc -o build\bin\data\shaders\%1-frag.bin ^
  --platform windows --profile 130 --type fragment --verbose -i ./ 
goto :eof

EXIT /B 0

