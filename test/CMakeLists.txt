# SDL_physfs_test
add_executable(SDL_PhysFS_Test SDL_PhysFS_Test.c)
target_compile_options(SDL_PhysFS_Test PRIVATE -Wall -Wextra -Wconversion -Wsign-conversion)
target_link_libraries(SDL_PhysFS_Test PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
    physfs-static
    SDL_PhysFS
)

# Resources
file(GLOB resources resources/*)
set(test_resources)
list(APPEND test_resources ${resources})
file(COPY ${test_resources} DESTINATION "resources/")

# Set up the test
list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
add_test(NAME SDL_PhysFS_Test COMMAND SDL_PhysFS_Test)
