@echo off

REM Commands to configure this repo and dependencies for building

call emcmake cmake -B build-em/debug-emscripten  ^
-DCMAKE_BUILD_TYPE=Debug -DSUPERBUILD=ON

call emcmake cmake -B build-em/release-emscripten  ^
-DCMAKE_BUILD_TYPE=Release -DSUPERBUILD=ON

PAUSE
