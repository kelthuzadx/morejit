cmake_minimum_required(VERSION 3.5)
project(morejit)

set(CMAKE_CXX_STANDARD 17)
set(SRC morejit.hpp instr_x86.hpp morejit.cpp morejit_x86.hpp test_x86.cpp type_x86.hpp)

add_executable(morejit ${SRC})