cmake_minimum_required(VERSION 3.24)
project(gl)

set(CMAKE_CXX_STANDARD 14)

add_executable(lab1 lab1.cpp)
target_link_libraries(lab1 PRIVATE glut GL GLU)

add_executable(lab2 lab2.cpp)
target_link_libraries(lab2 PRIVATE glut GL GLU)

add_executable(lab3 lab3.cpp)
target_link_libraries(lab3 PRIVATE glut GL GLU)

add_executable(lab4 lab4.cpp)
target_link_libraries(lab4 PRIVATE glut GL GLU)