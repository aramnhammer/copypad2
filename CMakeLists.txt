cmake_minimum_required(VERSION 3.27)
project(copypad2 C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
if(APPLE)
    enable_language(OBJC)
endif()
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

FetchContent_Declare(
    clay 
    GIT_REPOSITORY "https://github.com/nicbarker/clay.git"
    GIT_TAG "main"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)

FetchContent_GetProperties(clay)
if(NOT clay_POPULATED) # created by FetchContent_GetPropertiesa
    FetchContent_Populate(clay)
endif()

add_executable(copypad2 main.c)
target_link_libraries(copypad2 PUBLIC raylib)
target_include_directories(copypad2 PUBLIC 
    .
    ${clay_SOURCE_DIR}
)
if(APPLE)
    target_sources(copypad2 PRIVATE src/macos/macos_hotkeys.m)
    set_source_files_properties(src/macos_hotkeys.m PROPERTIES LANGUAGE OBJC)
    target_link_libraries(copypad2 PUBLIC "-framework AppKit" "-framework Carbon")
endif()
add_custom_command(
    TARGET copypad2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
    ${CMAKE_CURRENT_BINARY_DIR}/resources
    COMMENT "Copying resources..."
)