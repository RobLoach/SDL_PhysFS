# CMAKE Modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

# Dependencies
find_package(SDL3 REQUIRED)
find_package(PhysFS REQUIRED)

# Resources
file(GLOB resources resources/*)
set(examples_resources)
list(APPEND examples_resources ${resources})
file(COPY ${examples_resources} DESTINATION "resources/")

# SDL_physfs_example
add_executable(SDL_PhysFS_Example SDL_PhysFS_Example.c)
target_link_libraries(SDL_PhysFS_Example PRIVATE
    SDL3::SDL3-static
    physfs-static
    SDL_PhysFS
)
