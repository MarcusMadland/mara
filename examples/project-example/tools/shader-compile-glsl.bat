@echo off

echo Compiling shaders ...

REM Variables
set SHADERS=mesh

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
  echo Compiling vertex shader %1 for glsl ...
  bin\shaderc.exe ^
  -f ..\resources\shaders\%1\vs_%1.sc -o ..\runtime\shaders\glsl\vs_%1.bin ^
  --platform windows --profile 130 --type vertex --verbose -i ./ 
goto :eof

REM Function to compile a fragment shader
:compileFragmentShader
  echo Compiling fragment shader %1 for glsl ...
  bin\shaderc.exe ^
  -f ..\resources\shaders\%1\fs_%1.sc -o ..\runtime\shaders\glsl\fs_%1.bin ^
  --platform windows --profile 130 --type fragment --verbose -i ./ 
goto :eof

EXIT /B 0