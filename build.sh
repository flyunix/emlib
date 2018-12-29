#!/bin/bash

BUILD_TYPE="$1"
CFLAGS=
CMAKE_BUILD_TYPE="Release"

trap 'echo -e "\nrecive signal, exit.\n"; exit 0' INT QUIT TERM

git_checkin() 
{
    read -p "Want to checkin? [Y/N]?" cmd;
    if [[ "${cmd}" = "Y" ]]; then
        git add -A;
        read -p "Please input commit msg:" msg;
        git commit -m "${msg}";
    fi
}

if [[ -z "${BUILD_TYPE}" ]]; then
    BUILD_TYPE="TEST"
fi

if [[ "${BUILD_TYPE}" = "DEBUG" ]]; then
    CMAKE_BUILD_TYPE="Debug"
elif [[ "${BUILD_TYPE}" = "RELEASE" ]]; then
    CMAKE_BUILD_TYPE="Release"
elif [[ "${BUILD_TYPE}" = "TEST" ]]; then
    CMAKE_BUILD_TYPE="Debug"
    CFLAGS="-DNDEBUG"
fi

cmake . -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_C_FLAGS=${CFLAGS} && make clean && make 

if [[ "$?" != "0" ]]; then
    echo -e "make exec faild.\n"
    exit 1;
fi

if [[ -f emlib ]]; then
    ./emlib
    echo -e "\n\n"
    if [[ "$?" = "0" ]];then
        git_checkin
    fi
fi

