#!/bin/bash
cmake -S ./ -B ./build/ -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -G "Unix Makefiles" -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug
