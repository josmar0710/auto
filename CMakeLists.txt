cmake_minimum_required(VERSION 3.31)

project(auto CXX)

# set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

find_package(SDL3 REQUIRED CONFIG)
# find_package(SDL3_ttf REQUIRED CONFIG)

file(GLOB_RECURSE CC_SRC "cc/*cc")

add_executable(auto ${CC_SRC})

# target_link_options(p0 PRIVATE LINKER:-rpath,$ORIGIN/lib)

target_link_directories(auto PRIVATE "/usr/local/lib")

target_include_directories(auto PRIVATE "cc")

target_include_directories(auto SYSTEM PRIVATE "/usr/local/include")

target_link_libraries(auto PRIVATE wgpu_native SDL3::SDL3)
