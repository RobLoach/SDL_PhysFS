# CMAKE Modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# The header is compiled as C++ as well, to make sure it stays valid C++.
enable_language(CXX)

# Dependencies
find_package(SDL3 REQUIRED)
find_package(PhysFS REQUIRED)

# Warnings applied to every test target. -Wstrict-prototypes is C only, and
# catches unprototyped declarations like `bool SDL_PhysFS_Quit();`.
set(SDL_PhysFS_TestWarnings
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall;-Wextra;-Wconversion;-Wsign-conversion>
    $<$<C_COMPILER_ID:MSVC>:/W4>
    $<$<AND:$<NOT:$<C_COMPILER_ID:MSVC>>,$<COMPILE_LANGUAGE:C>>:-Wstrict-prototypes>
)

# SDL_physfs_test
add_executable(SDL_PhysFS_Test
    SDL_PhysFS_Test.c
)

# Compile checks: each one builds the header a different way, and makes a few
# calls so that the linker has to resolve them.
add_executable(SDL_PhysFS_Test_MultiTU
    compile/implementation.c
    compile/header_only.c
)

add_executable(SDL_PhysFS_Test_CXX
    compile/implementation.cpp
    compile/header_only.cpp
)

add_executable(SDL_PhysFS_Test_Static
    compile/static.c
)

# In SDL_PHYSFS_STATIC mode the whole API is internal to the one translation
# unit, so anything this check does not call is legitimately unused.
target_compile_options(SDL_PhysFS_Test_Static PRIVATE
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wno-unused-function>
)

foreach(target
    SDL_PhysFS_Test
    SDL_PhysFS_Test_MultiTU
    SDL_PhysFS_Test_CXX
    SDL_PhysFS_Test_Static
)
    target_compile_options(${target} PRIVATE ${SDL_PhysFS_TestWarnings})
    target_link_libraries(${target} PRIVATE
        SDL3::SDL3-static
        physfs-static
        SDL_PhysFS
    )
    add_test(NAME ${target} COMMAND ${target})
endforeach()

# Resources
file(GLOB resources resources/*)
set(test_resources)
list(APPEND test_resources ${resources})
file(COPY ${test_resources} DESTINATION "resources/")

# Set up the tests
list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
