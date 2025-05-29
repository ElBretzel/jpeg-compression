cmake_minimum_required(VERSION 3.20)
project(JPEG_PROJECT LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=znver4 -flto -DNDEBUG -fno-rtti -fno-asynchronous-unwind-tables -pthread")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -fno-math-errno -fdiagnostics-color=always")
endif()

find_package(OpenGL REQUIRED)
find_package(xsimd REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(HWLOC REQUIRED hwloc)

include(FetchContent)

FetchContent_Declare(
  SFML
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG 3.0.1
  GIT_SHALLOW ON
)
FetchContent_MakeAvailable(SFML)

FetchContent_Declare(
  ImGui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.1
  GIT_SHALLOW ON
)
FetchContent_MakeAvailable(ImGui)
FetchContent_GetProperties(ImGui SOURCE_DIR IMGUI_DIR)

FetchContent_Declare(
  ImGui-SFML
  GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
  GIT_TAG v3.0
  GIT_SHALLOW ON
)
set(IMGUI_SFML_BUILD_EXAMPLES OFF CACHE BOOL "Disable ImGui-SFML examples" FORCE)
set(IMGUI_SFML_BUILD_TESTS OFF CACHE BOOL "Disable ImGui-SFML tests" FORCE)
FetchContent_MakeAvailable(ImGui-SFML)
FetchContent_Declare(
  ImGui-FILEBROWSER
  GIT_REPOSITORY https://github.com/AirGuanZ/imgui-filebrowser.git
  GIT_TAG origin/master
  GIT_SHALLOW ON
)
FetchContent_MakeAvailable(ImGui-FILEBROWSER)
FetchContent_GetProperties(ImGui-FILEBROWSER SOURCE_DIR IMGUI_FILEBROWSER_SOURCE_DIR)

set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(PROJECT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(PROJECT_SOURCE_MENU_DIR ${PROJECT_SOURCE_DIR}/menu)
set(PROJECT_INCLUDE_MENU_DIR ${PROJECT_INCLUDE_DIR}/menu)

add_executable(a.out
    ${PROJECT_SOURCE_DIR}/main.cpp
    ${PROJECT_SOURCE_DIR}/jpeg_header.cpp
    ${PROJECT_SOURCE_DIR}/jpeg_body.cpp
    ${PROJECT_SOURCE_DIR}/jpeg_def.cpp
    ${PROJECT_SOURCE_DIR}/sprite_handler.cpp
    ${PROJECT_SOURCE_MENU_DIR}/frame_counter.cpp
    ${PROJECT_SOURCE_MENU_DIR}/file_browser.cpp

)

target_include_directories(a.out PRIVATE
    ${HWLOC_INCLUDE_DIRS}
    ${IMGUI_FILEBROWSER_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${PROJECT_INCLUDE_DIR}
    ${PROJECT_INCLUDE_MENU_DIR}
    
)

target_link_libraries(a.out PRIVATE
    xsimd
    ${HWLOC_LIBRARIES}
    OpenGL::GL
    ImGui-SFML::ImGui-SFML
    SFML::Graphics
)
