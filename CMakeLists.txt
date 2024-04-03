cmake_minimum_required(VERSION 3.12) # version can be different

project(AccessControl) #name of your project

find_package(Boost REQUIRED)


set(CMAKE_CXX_STANDARD 20)

set(SOURCES PVSS.h 
            PVSS/PVSS.cpp 
            PVSS/ConnectionHandler.h
            PVSS/ConnectionHandler.cpp
            PVSS/PVSSServer.h
            PVSS/PVSSServer.cpp
            DPF/gsl/gsl-lite.hpp
            DPF/AES.cpp
            DPF/AES.h
            DPF/Defines.cpp
            DPF/dpf.cpp
            DPF/dpf.h
            DPF/Log.cpp
            DPF/Log.h
            DPF/PRNG.cpp
            DPF/PRNG.h
            Network/Client.cpp
            Network/Client.h
            Network/Server.cpp
            Network/Server.h
            SSSharing/SSSharing.h
            PVSS/helperFunctions.h
            PACL/DataServerProtocol.h
            PACL/DataServerProtocol.cpp)

set(CMAKE_C_FLAGS "-ffunction-sections -Wall  -maes -msse2 -msse4.1 -mavx2 -mpclmul -Wfatal-errors -pthread -Wno-strict-overflow  -fPIC -Wno-ignored-attributes")
set(CMAKE_CXX_FLAGS  "${CMAKE_C_FLAGS}")


file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/riscZeroProver/accessControlUser)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/riscZeroProver/dataServersPublicKey)

# Copy folder to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ZeroKnowledgeProverCode/AccessControl/target/release/prover DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/riscZeroProver/accessControlUser)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ZeroKnowledgeProverCode/AccessControl/target/release/verifier DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/riscZeroProver/accessControlUser)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ZeroKnowledgeProverCode/DataServerPublicKey/target/release/prover DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/riscZeroProver/dataServersPublicKey)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ZeroKnowledgeProverCode/DataServerPublicKey/target/release/verifier DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/riscZeroProver/dataServersPublicKey)
# Create a new folders in the build directory for the data servers
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ds1_data)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ds2_data)


add_executable(${PROJECT_NAME} ${SOURCES} main.cpp)

target_link_libraries(${PROJECT_NAME} pthread ntl m gmp crypto)
