cmake_minimum_required(VERSION 3.10)
project(TextureSynthesis)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(TextureSynthesis main.cpp texturize.cpp graph.cpp maxflow.cpp graphcut.cpp)
target_link_libraries(TextureSynthesis ${OpenCV_LIBS})