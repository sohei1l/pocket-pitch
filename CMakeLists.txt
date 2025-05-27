cmake_minimum_required(VERSION 3.16)
project(pocket-pitch)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(RTAUDIO REQUIRED rtaudio)

add_executable(pocket-pitch src/main.cpp)

target_link_libraries(pocket-pitch ${RTAUDIO_LIBRARIES})
target_include_directories(pocket-pitch PRIVATE ${RTAUDIO_INCLUDE_DIRS})
target_compile_options(pocket-pitch PRIVATE ${RTAUDIO_CFLAGS_OTHER})