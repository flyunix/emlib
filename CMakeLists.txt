#Cmake最低版本要求
cmake_minimum_required(VERSION 3.1)

#项目名称
project(emlib)

#Library version
set(VERSION "2.0.0")
set(SOVERSION ${VERSION})

#option(CMAKE_BUILD_TYPE "Default for CMAKE_BUILD_TYPE" Release)
#message(STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})

#option(CMAKE_BUILD_TARGET "Default for CMAKE_BUILD_TARGET" LIBS)
#message(STATUS "CMAKE_BUILD_TARGET: " ${CMAKE_BUILD_TARGET})

#add_compile_options(-std=gnu99 -Werror)
#set(CMAKE_CFLAGS_DEBUG    "$ENV{CFLAGS} -O0 -Wall -g -ggdb -fstack-protector-all")
#set(CMAKE_CFLAGS_RELEASE  "$ENV{CFLAGS} -O3 -Wall -DNDEBUG")

#查找src目录下的所有源文件,将其存储到DIR_SRCS变量中
#aux_source_directory(./src DIR_SRCS)

#if(${CMAKE_BUILD_TARGET} STREQUAL "EXE") 
#    aux_source_directory(./test DIR_SRCS)
#endif(${CMAKE_BUILD_TARGET} STREQUAL "EXE")

#配置头文件存放的目录
#include_directories(./include)

#生成目标
#if(${CMAKE_BUILD_TARGET} STREQUAL "LIBS") 
#    add_library(${PROJECT_NAME} SHARED ${DIR_SRCS})
#elseif(${CMAKE_BUILD_TARGET} STREQUAL "EXE")
#add_executable(${PROJECT_NAME} ${DIR_SRCS})
#endif(${CMAKE_BUILD_TARGET} STREQUAL "LIBS")

# Glob sources, headers, tests
file(GLOB_RECURSE emlib_SOURCES src/*.c)
file(GLOB_RECURSE emlib_em_HEADERS include/em/*.h)
file(GLOB_RECURSE emlib_HEADERS include/emlib.h)
file(GLOB_RECURSE emlib_TESTS   tests/*.c)

# Expose git commit id into COMMIT_ID variable
execute_process(
    COMMAND git describe --abbrev --always --tags --dirty
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE COMMIT_ID
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Define C flags and include directories
add_definitions(-DPERIPHERY_VERSION_COMMIT="${COMMIT_ID}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -fstack-protector-all -Wall -Wno-unused-parameter -fPIC")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Declare library target
add_library(${PROJECT_NAME} ${emlib_SOURCES})
set_target_properties(emlib PROPERTIES SOVERSION ${VERSION})
target_link_libraries(${PROJECT_NAME} pthread rt)

# Declare install targets
install(TARGETS emlib DESTINATION lib)
install(DIRECTORY include/em DESTINATION include)
install(FILES ${emlib_HEADERS} DESTINATION include)

# Declare tests targets
# foreach(TEST_SOURCE ${emlib_TESTS})
#   get_filename_component(TEST_PROGRAM ${TEST_SOURCE} NAME_WE)
#    add_executable(${TEST_PROGRAM} ${TEST_SOURCE})
#    target_link_libraries(${TEST_PROGRAM} emlib pthread)
#    set(TEST_PROGRAMS ${TEST_PROGRAMS} ${TEST_PROGRAM})
#endforeach()
#add_custom_target(tests DEPENDS emlib ${TEST_PROGRAMS})
