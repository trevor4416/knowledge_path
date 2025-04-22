cmake_minimum_required(VERSION 3.29.6)
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

#pulling in glfw (imgui backend)
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.3.8
)
FetchContent_MakeAvailable(glfw)

# make sure your OpenSSL installation's root directory is at the path specified in the line below.
# Install using https://slproweb.com/download/Win64OpenSSL-3_5_0.exe with the default options.
set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
set(OPENSSL_LIBRARIES "${OPENSSL_ROOT_DIR}/lib/libssl.lib" "${OPENSSL_ROOT_DIR}/lib/libcrypto.lib")

find_package(OpenSSL REQUIRED)

include_directories(src test lib ${OPENSSL_INCLUDE_DIR})

find_package(OpenGL REQUIRED)
#find_package(SDL2 REQUIRED)

file(GLOB IMGUI_SOURCES
        extern/imgui/*.cpp
        extern/imgui/backends/imgui_impl_glfw.cpp
        extern/imgui/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES}
        src/main.cpp)
target_include_directories(imgui PUBLIC
        extern/imgui
        extern/imgui/backends
)
target_link_libraries(imgui PUBLIC OpenGL::GL)

add_executable(Main
        src/main.cpp
        lib/cpp-httplib/httplib.h
        lib/json/single_include/nlohmann/json.hpp
        src/openalex.cpp
        src/openalex.h
        src/Graph.cpp
        src/Graph.h
        src/Graph.h
        src/Graph.h
        src/Graph.h
        src/main.cpp
)

add_executable(Tests
        test/test.cpp
        lib/cpp-httplib/httplib.h
        lib/json/single_include/nlohmann/json.hpp
        src/openalex.cpp
        src/openalex.h
        src/Graph.h
        src/Graph.h
        src/main.cpp
)
target_include_directories(Main PRIVATE
        extern/imgui
        extern/imgui/backends
        ${glfw_SOURCE_DIR}/include
)
target_link_libraries(Main PRIVATE ${OPENSSL_LIBRARIES} ws2_32 crypt32)
target_link_libraries(Tests PRIVATE Catch2::Catch2WithMain ${OPENSSL_LIBRARIES} ws2_32 crypt32)
target_link_libraries(Main PRIVATE imgui OpenGL::GL)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)
target_link_libraries(Main PRIVATE imgui)