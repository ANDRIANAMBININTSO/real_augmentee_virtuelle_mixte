cmake_minimum_required(VERSION 3.16)
project(monprojet)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(glfw3 REQUIRED)

add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

add_executable(main main.cpp)
target_include_directories(main PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(main PRIVATE glad glfw ${OpenCV_LIBS} ${CMAKE_DL_LIBS})