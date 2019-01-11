#Cmake 最低版本要求
cmake_minimum_required(VERSION 3.1)

#项目名称
project(emlib)

add_compile_options(-std=gnu99 -Werror)
set(CMAKE_CFLAGS_DEBUG    "$ENV{CFLAGS} -O0 -Wall -g -ggdb -fstack-protector-all")
set(CMAKE_CFLAGS_RELEASE  "$ENV{CFLAGS} -O3 -Wall -DNDEBUG")

#查找src目录下的所有源文件,将其存储到DIR_SRCS变量中
aux_source_directory(./src DIR_SRCS)
aux_source_directory(./test DIR_SRCS)

#配置头文件存放的目录
include_directories(./include)

#生成制定目标
add_executable(${PROJECT_NAME} ${DIR_SRCS})

#配置链接库
target_link_libraries(${PROJECT_NAME} pthread)
