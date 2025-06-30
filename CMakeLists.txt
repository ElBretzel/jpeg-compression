cmake_minimum_required(VERSION 3.20)
project(JPEG_PROJECT LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -march=native \
    -fno-rtti -fno-exceptions \
    -funsafe-math-optimizations -ffinite-math-only -fno-math-errno \
    -fno-trapping-math -ffp-contract=fast \
    -funroll-loops -ftree-vectorize -fprefetch-loop-arrays \
    -mavx2 -mfma -mbmi -mbmi2 -mlzcnt -mpopcnt -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -fno-math-errno -fdiagnostics-color=always -fsanitize=address,undefined")
endif()

set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(PROJECT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

add_executable(a.out
    ${PROJECT_SOURCE_DIR}/main.cpp
    ${PROJECT_SOURCE_DIR}/huffman_code.cpp
    ${PROJECT_SOURCE_DIR}/jpeg_header.cpp
    ${PROJECT_SOURCE_DIR}/jpeg_scan.cpp
    ${PROJECT_SOURCE_DIR}/quantization.cpp
    ${PROJECT_SOURCE_DIR}/dct.cpp
    ${PROJECT_SOURCE_DIR}/color_conversion.cpp
    ${PROJECT_SOURCE_DIR}/jpeg_data_stream.cpp

)

target_include_directories(a.out PRIVATE
    ${HWLOC_INCLUDE_DIRS}
    ${PROJECT_INCLUDE_DIR}
    ${PROJECT_INCLUDE_MENU_DIR}
    
)

target_link_libraries(a.out PRIVATE
)
