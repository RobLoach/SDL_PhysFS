# SDL_physfs
add_library(SDL_physfs INTERFACE)
target_include_directories(SDL_physfs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
install(FILES
    SDL_physfs.h
    DESTINATION include
)