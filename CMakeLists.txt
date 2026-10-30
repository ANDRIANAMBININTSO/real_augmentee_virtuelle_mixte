cmake_minimum_required(VERSION 3.5)
project(main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenCV REQUIRED)



add_executable(main 
    src/detection.cpp
    src/video.cpp
    src/main.cpp
)

target_include_directories(main PRIVATE
    ${CMAKE_SOURCE_DIR}/src/
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(main glfw)
target_link_libraries(main OpenGL::GL)
target_link_libraries(main GLEW::GLEW)
target_link_libraries(main ${OpenCV_LIBS})