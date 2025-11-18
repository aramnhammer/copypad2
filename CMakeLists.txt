cmake_minimum_required(VERSION 3.27)
project(copypad2 C)
enable_language(OBJC)
set(CMAKE_C_STANDARD 99)

# Adding Raylib
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "5.5"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(raylib)

add_executable(copypad2 main.c)

target_compile_options(copypad2 PUBLIC)
target_include_directories(copypad2 PUBLIC .)

if(APPLE)
    set_source_files_properties(src/macos_hotkeys.m PROPERTIES LANGUAGE OBJC)
    target_sources(copypad2 PRIVATE src/macos_hotkeys.m)
    target_link_libraries(copypad2 PUBLIC raylib "-framework AppKit" "-framework Carbon")
else()
    target_link_libraries(copypad2 PUBLIC raylib)
endif()

# Copy resources
add_custom_command(
    TARGET copypad2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
    ${CMAKE_CURRENT_BINARY_DIR}/resources
)