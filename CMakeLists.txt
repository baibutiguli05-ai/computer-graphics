cmake_minimum_required(VERSION 3.10)
project(ComputerGraphics CXX C)

set(CMAKE_CXX_STANDARD 17)
cmake_policy(SET CMP0072 NEW)

find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)

include_directories(
    ${CMAKE_SOURCE_DIR}/external/glad/include
    ${CMAKE_SOURCE_DIR}/external/stb
    ${GLFW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)

file(GLOB_RECURSE GLAD_SOURCES "external/glad/src/*.c" "external/glad/src/*.cpp")
file(GLOB SRCS "*.cpp")

add_executable(main ${SRCS} ${GLAD_SOURCES})

target_link_libraries(main
    ${GLFW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    dl
    pthread
)