# SDL_physfs_test
add_executable(SDL_physfs_test SDL_physfs_test.c)
target_compile_options(SDL_physfs_test PRIVATE -Wall -Wextra -Wconversion -Wsign-conversion)
target_link_libraries(SDL_physfs_test PUBLIC
    ${SDL2_LIBRARIES}
    physfs-static
    SDL_physfs
)
target_include_directories(SDL_physfs_example PUBLIC ${SDL2_INCLUDE_DIRS})

# Resources
file(GLOB resources resources/*)
set(test_resources)
list(APPEND test_resources ${resources})
file(COPY ${test_resources} DESTINATION "resources/")

# Set up the test
list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
add_test(NAME SDL_physfs_example COMMAND SDL_physfs_example)