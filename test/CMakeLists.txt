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

# Integration checks: expand each optional integration macro against the real
# header, so drift from the library's API fails the build. They are compile
# only, and each is skipped when its header is not available.
foreach(lib image mixer ttf)
    set(include_dir)
    find_package(SDL3_${lib} CONFIG QUIET)
    if (TARGET SDL3_${lib}::SDL3_${lib})
        set(include_dir $<TARGET_PROPERTY:SDL3_${lib}::SDL3_${lib},INTERFACE_INCLUDE_DIRECTORIES>)
    else()
        find_path(SDL3_${lib}_INCLUDE_DIR SDL3_${lib}/SDL_${lib}.h)
        if (SDL3_${lib}_INCLUDE_DIR)
            set(include_dir ${SDL3_${lib}_INCLUDE_DIR})
        endif()
    endif()
    if (include_dir)
        list(APPEND SDL_PhysFS_Integrations ${lib})
        set(SDL_PhysFS_Integration_${lib}_Include ${include_dir})
    else()
        message(STATUS "SDL3_${lib} not found, skipping its integration check")
    endif()
endforeach()
set(SDL_PhysFS_Integration_image_Source compile/img.c)
set(SDL_PhysFS_Integration_mixer_Source compile/mix.c)
set(SDL_PhysFS_Integration_ttf_Source compile/ttf.c)

find_path(SDL_STBIMAGE_INCLUDE_DIR SDL_stbimage.h)
find_path(STB_IMAGE_INCLUDE_DIR stb_image.h)
if (SDL_STBIMAGE_INCLUDE_DIR AND STB_IMAGE_INCLUDE_DIR)
    list(APPEND SDL_PhysFS_Integrations stbimage)
    set(SDL_PhysFS_Integration_stbimage_Include ${SDL_STBIMAGE_INCLUDE_DIR} ${STB_IMAGE_INCLUDE_DIR})
    set(SDL_PhysFS_Integration_stbimage_Source compile/stbimage.c)
else()
    message(STATUS "SDL_stbimage.h or stb_image.h not found, skipping its integration check")
endif()

foreach(lib IN LISTS SDL_PhysFS_Integrations)
    add_library(SDL_PhysFS_Test_Integration_${lib} OBJECT ${SDL_PhysFS_Integration_${lib}_Source})
    target_include_directories(SDL_PhysFS_Test_Integration_${lib} PRIVATE ${SDL_PhysFS_Integration_${lib}_Include})
    target_compile_options(SDL_PhysFS_Test_Integration_${lib} PRIVATE
        ${SDL_PhysFS_TestWarnings}
        $<IF:$<C_COMPILER_ID:MSVC>,/WX,-Werror>
    )
    target_link_libraries(SDL_PhysFS_Test_Integration_${lib} PRIVATE
        SDL3::SDL3-static
        SDL_PhysFS
    )
endforeach()

# Resources
file(GLOB resources resources/*)
set(test_resources)
list(APPEND test_resources ${resources})
file(COPY ${test_resources} DESTINATION "resources/")

# Set up the tests
list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
