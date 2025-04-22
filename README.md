# Knowledge Path
Find the shortest citation path between two academic papers using OpenAlex data.

## Prerequisites
- **C++20** compiler (GCC ≥ 10, Clang ≥ 11, MSVC 2019+)  
- **CMake** ≥ 3.16  
- **OpenSSL 3.5+**  
  - Windows installer: https://slproweb.com/products/Win32OpenSSL.html  
  - Install to `C:/Program Files/OpenSSL-Win64`
  - If you see runtime errors about missing DLLs, copy the two files below into your app’s working directory (or add that folder to your `PATH`):
    - `C:/Program Files/OpenSSL-Win64/bin/libssl-3.dll`
    - `C:/Program Files/OpenSSL-Win64/bin/libcrypto-3.dll`
- **GLFW** (https://www.glfw.org)  
- **OpenGL** (provided by your graphics driver)
- **ImGui** (automatically fetched by CMake)  
- **cpp-httplib** and **nlohmann/json** (bundled as submodules)

## Clone & Build
```bash
git clone https://github.com/trevor4416/knowledge_path.git
cd knowledge_path
mkdir build && cd build

# MinGW Makefiles:
cmake -G "MinGW Makefiles" .. \
  -DOPENSSL_ROOT_DIR="C:/Program Files/OpenSSL-Win64"

# Or with Ninja:
cmake -G Ninja .. \
  -DOPENSSL_ROOT_DIR="C:/Program Files/OpenSSL-Win64"

cmake --build .
