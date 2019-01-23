#!/bin/bash


BUILD_TYPE="$1"
CFLAGS=
CMAKE_BUILD_TYPE="Release"

#trap 'echo -e "\nreceive signal, exit.\n"; exit 0' INT QUIT TERM

git_checkin() 
{
    read -p "Want to checkin? [Y/N]?" cmd;
    if [[ "${cmd}" = "Y" || "${cmd}" = "y" ]]; then
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
    CFLAGS="-DNASSERT"
fi

#set C/C++ compiler
#COMPILE_PATH="-DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/g++ -DCMAKE_C_COMPILER:FILEPATH=arm-none-linux-gnueabi-gcc"
COMPILE_PATH="-DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/g++ -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/gcc"

cmake . $COMPILE_PATH -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_C_FLAGS=${CFLAGS} && make clean && make 

if [[ "$?" != "0" ]]; then
    echo -e "\n\e[1;31m MAKE FAILD. \e[0m \n"
    exit 1;
fi

echo -e "\n\e[1;32m Build SUCC.\e[0m \n"

if [[ -f emlib ]]; then
    ulimit -c unlimited
    ./emlib 3

    RET=$?
    echo -e "\n\n"
    if [[ RET -eq 0 ]];then
        echo -e "\n\e[1;32m Test Cases Run Succ.\e[0m \n"
        git_checkin
    else
        echo -e "\n\e[1;31m Test Cases Run Failed.\e[0m \n"
    fi
fi

