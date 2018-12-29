#!/bin/bash

BUILD_TYPE="$1"
CFLAGS=
CMAKE_BUILD_TYPE="Release"

if [ -n "${BUILD_TYPE}" ]; then
    if [ "${BUILD_TYPE}" = "DEBUG" ]; then
        CMAKE_BUILD_TYPE="Debug"
    elif [ "${BUILD_TYPE}" = "RELEASE" ]; then
        CMAKE_BUILD_TYPE="Release"
    elif [ "${BUILD_TYPE}" = "TEST" ]; then
        CMAKE_BUILD_TYPE="Debug"
        CFLAGS="-DNDEBUG"
    fi
fi

cmake . -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_C_FLAGS=${CFLAGS} && make clean && make 
