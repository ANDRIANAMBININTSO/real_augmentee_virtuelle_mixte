cmake_minimum_required(VERSION 3.5)
project(main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/glm)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(main main.cpp)

target_link_libraries(main glfw)
target_link_libraries(main OpenGL::GL)
target_link_libraries(main GLEW::GLEW)
target_link_libraries(main ${OpenCV_LIBS})