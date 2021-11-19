cmake_minimum_required(VERSION 3.16)

project(qkamber)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(qkamber ${SOURCES})
target_include_directories(qkamber PRIVATE "src")
target_precompile_headers(qkamber PRIVATE "src/precompiled.h")

# copy assets
file(GLOB ASSETS "assets/*")
file(COPY ${ASSETS} DESTINATION .)
