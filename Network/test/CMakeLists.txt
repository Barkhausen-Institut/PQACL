cmake_minimum_required(VERSION 3.12) # version can be different

project(threadTest) #name of your project

find_package(Boost REQUIRED)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES ../Client.h ../Client.cpp ../Server.h ../Server.cpp ../main.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

# add_executable(threadTest main.cpp Server.h Server.cpp Client.h Client.cpp ) # add this executable

target_link_libraries(${PROJECT_NAME} pthread boost_program_options)

# target_link_libraries(threadTest PRIVATE Threads::Threads)
# target_link_libraries(threadTest PRIVATE Boost::boost)

# for the googletests:

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
set(TEST_SOURCES ../Client.h ../Client.cpp ../Server.h ../Server.cpp tests.cpp)

add_executable(run_tests ${TEST_SOURCES})
target_link_libraries(run_tests gtest_main pthread boost_program_options)
include(GoogleTest)
gtest_discover_tests(run_tests)

