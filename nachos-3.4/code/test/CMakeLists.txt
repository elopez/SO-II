cmake_minimum_required(VERSION 2.8.11)
project(tests ASM C)

include_directories(../threads ../userprog)

# Force CMake to use our compiler binaries
include(CMakeForceCompiler)
cmake_force_c_compiler(mips-dec-ultrix42/mips-gcc GNU)
set(CMAKE_C_LINK_EXECUTABLE "mips-dec-ultrix42/mips-ld <FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET>")
set(CMAKE_ASM_COMPILER mips-dec-ultrix42/mips-gcc)

# Special magic flags needed to build mips binaries
set(CMAKE_C_FLAGS "-G 0 -mips1")
set(CMAKE_ASM_FLAGS "-mips1")
set(CMAKE_EXE_LINKER_FLAGS "-T script -N")

# Basic set of code on all userspace programs
add_library(stdlib OBJECT start.S minilib.c)

# Pick up all *.c files, except our stdlib
file(GLOB TESTS "*.c")
list(REMOVE_ITEM TESTS "${CMAKE_CURRENT_SOURCE_DIR}/minilib.c")

# the coff2noff tool is built outside
if(NOT TARGET coff2noff)
    add_executable(coff2noff IMPORTED)
endif()

# And make a binary out of each one
foreach(test ${TESTS})
    get_filename_component(TEST_NAME ${test} NAME_WE)
    add_executable(${TEST_NAME} $<TARGET_OBJECTS:stdlib> ${test})
    add_dependencies(${TEST_NAME} coff2noff)
    add_custom_command(TARGET ${TEST_NAME} POST_BUILD
        COMMAND mv ${TEST_NAME} ${TEST_NAME}.coff
        COMMAND ../bin/coff2noff ${TEST_NAME}.coff ${TEST_NAME}
        COMMAND rm ${TEST_NAME}.coff)
endforeach(test)
