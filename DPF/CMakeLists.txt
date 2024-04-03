cmake_minimum_required(VERSION 3.14)
project(DPFPIR)

set(CMAKE_CXX_STANDARD 20)

set(SRCS
    AES.cpp
    Defines.cpp
    Log.cpp
    PRNG.cpp
    dpf.cpp)

set(CMAKE_C_FLAGS "-ffunction-sections -Wall  -maes -msse2 -msse4.1 -mavx2 -mpclmul -Wfatal-errors -pthread -Wno-strict-overflow  -fPIC -Wno-ignored-attributes")
set(CMAKE_CXX_FLAGS  "${CMAKE_C_FLAGS}")

# Select flags.
#SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
#SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO " -O2 -g -ggdb -rdynamic")
# SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g -ggdb -rdynamic")

add_executable(my_dpf ${SRCS} my_main.cpp)
target_link_libraries(my_dpf crypto)

