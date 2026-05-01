# CMAKE Modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Dependencies
find_package(SDL3 REQUIRED)
find_package(PhysFS REQUIRED)

# SDL_physfs_test
add_executable(SDL_PhysFS_Test SDL_PhysFS_Test.c)
target_compile_options(SDL_PhysFS_Test PRIVATE
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall;-Wextra;-Wconversion;-Wsign-conversion>
    $<$<C_COMPILER_ID:MSVC>:/W4>
)
target_link_libraries(SDL_PhysFS_Test PRIVATE
    SDL3::SDL3-static
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
