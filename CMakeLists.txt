cmake_minimum_required(VERSION 3.16)
project(wasm_lightning)

set(CMAKE_CXX_FLAGS "-O3 --std=c++17 -s USE_SDL=2 -s -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -o index.html")


set(CMAKE_EXECUTABLE_SUFFIX ".js")
add_executable(wasm_lightning main.cpp app.cpp maze.cpp)
set_property(TARGET wasm_lightning PROPERTY CXX_STANDARD 17)

add_custom_command(TARGET wasm_lightning POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/index.html ${CMAKE_CURRENT_BINARY_DIR}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/style.css ${CMAKE_CURRENT_BINARY_DIR}/
)