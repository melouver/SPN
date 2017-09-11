cmake_minimum_required(VERSION 3.8)
project(SPN_9_9)

set(CMAKE_CXX_STANDARD 11)

set(SPN_SRC spn.cpp)
set(RSA_SRC RSA.cpp)

add_executable(spn ${SPN_SRC})


add_executable(rsa ${RSA_SRC})
target_link_libraries(rsa gmp)
