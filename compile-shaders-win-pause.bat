@echo off

echo Compiling shaders ...

REM Variables
set PLATFORM = windows
set API = s_5_0
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
  tools\shaderc.exe ^
  -f mrender\shaders\src\%1\%1-vert.sc -o mrender\shaders\build\%1-vert.bin ^
  --platform %PLATFORM% --type vertex --verbose -i ./ -p %API%
goto :eof

REM Function to compile a fragment shader
:compileFragmentShader
  echo Compiling fragment shader %1 ...
  tools\shaderc.exe ^
  -f mrender\shaders\src\%1\%1-frag.sc -o mrender\shaders\build\%1-frag.bin ^
  --platform %PLATFORM% --type fragment --verbose -i ./ -p %API%
goto :eof
EXIT /B 0

