cmake_minimum_required(VERSION 3.16)

project(qkamber)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

# include the specific platform
list(FILTER SOURCES EXCLUDE REGEX "src/platform")
set(PLATFORM "sdl")
if(WINDOWS)
    set(PLATFORM "win32")
endif()

file(GLOB_RECURSE PLATFORM_SOURCES CONFIGURE_DEPENDS "src/platform/${PLATFORM}/*.cpp")
list(APPEND SOURCES ${PLATFORM_SOURCES})

add_executable(qkamber ${SOURCES})
target_include_directories(qkamber PRIVATE "src")
target_precompile_headers(qkamber PRIVATE "src/precompiled.h")

if(NOT WINDOWS)
    add_subdirectory("extern/sdl")
    target_include_directories(qkamber PRIVATE "extern/sdl/include")
    target_link_libraries(qkamber PRIVATE SDL2)
endif()

target_compile_options(qkamber PRIVATE "-O2")

# copy assets
file(GLOB ASSETS "assets/*")
file(COPY ${ASSETS} DESTINATION .)
