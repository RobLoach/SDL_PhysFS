cmake_minimum_required(VERSION 3.7)

project(SDL_physfs)

find_package(SDL2 REQUIRED)

add_executable(SDL_physfs_example example/main.cpp)
target_link_libraries(SDL_physfs_example ${SDL2_LIBRARIES})
target_include_directories(SDL_physfs_example ${SDL2_INCLUDE_DIRS})

add_library(SDL_physfs)