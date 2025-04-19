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

# make sure your OpenSSL installation's root directory is at the path specified in the line below.
# Install using https://slproweb.com/download/Win64OpenSSL-3_5_0.exe with the default options.
set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
set(OPENSSL_LIBRARIES "${OPENSSL_ROOT_DIR}/lib/libssl.lib" "${OPENSSL_ROOT_DIR}/lib/libcrypto.lib")

find_package(OpenSSL REQUIRED)

include_directories(src test lib ${OPENSSL_INCLUDE_DIR})

add_executable(Main
        src/main.cpp
        lib/cpp-httplib/httplib.h
        lib/json/single_include/nlohmann/json.hpp
        src/openalex.cpp
        src/openalex.h
)

add_executable(Tests
        test/test.cpp
        lib/cpp-httplib/httplib.h
        lib/json/single_include/nlohmann/json.hpp
        src/openalex.cpp
        src/openalex.h
)
target_link_libraries(Main PRIVATE ${OPENSSL_LIBRARIES} ws2_32 crypt32)
target_link_libraries(Tests PRIVATE Catch2::Catch2WithMain ${OPENSSL_LIBRARIES} ws2_32 crypt32)