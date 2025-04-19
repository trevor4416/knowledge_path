cmake_minimum_required(VERSION 3.30)
project(knowledge_path)

set(CMAKE_CXX_STANDARD 20)

set(GCC_COVERAGE_COMPILE_FLAGS "-Wall -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )

##pull in desired version of catch through cmake automatically, make it available
Include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

include_directories(src test)

add_executable(Main
        src/main.cpp
        src/httplib.h src/json.hpp
)

add_executable(Tests
        test/test.cpp
        src/httplib.h src/json.hpp
)

target_link_libraries(Tests PRIVATE Catch2::Catch2WithMain)