#!/bin/bash

# Commands to configure this repo and dependencies for building

emcmake cmake -B build-em/debug-emscripten -G Ninja \
-DCMAKE_BUILD_TYPE=Debug -DSUPERBUILD=ON

emcmake cmake -B build-em/release-emscripten -G Ninja \
-DCMAKE_BUILD_TYPE=Release -DSUPERBUILD=ON
