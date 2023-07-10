# CMAKE Modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

set(SDL_SHARED FALSE)
set(SDL2_DISABLE_UNINSTALL TRUE)

# Dependencies
find_package(SDL2 REQUIRED)
find_package(PhysFS REQUIRED)

# Resources
file(GLOB resources resources/*)
set(examples_resources)
list(APPEND examples_resources ${resources})
file(COPY ${examples_resources} DESTINATION "resources/")

# SDL_physfs_example
add_executable(SDL_PhysFS_Example SDL_PhysFS_Example.c)
target_link_libraries(SDL_PhysFS_Example PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
    physfs-static
    SDL_PhysFS
)
