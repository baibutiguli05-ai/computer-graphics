cmake_minimum_required(VERSION 3.10)
project(ComputerGraphics CXX C)

set(CMAKE_CXX_STANDARD 17)

# 查找 Linux 系统的 OpenGL 和 GLFW 库
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)

# 包含头文件路径
include_directories(
    ${CMAKE_SOURCE_DIR}/external/glad/include
    ${CMAKE_SOURCE_DIR}/external/stb
    ${GLFW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)

# 自动查找 GLAD 的 C/CPP 源文件
file(GLOB_RECURSE GLAD_SOURCES "external/glad/src/*.c" "external/glad/src/*.cpp")

# 自动查找根目录下所有 .cpp 源文件
file(GLOB SRCS "*.cpp")

# 生成可执行程序
add_executable(main ${SRCS} ${GLAD_SOURCES})

# 链接 Linux 系统图形库
target_link_libraries(main
    ${GLFW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    dl
    pthread
)