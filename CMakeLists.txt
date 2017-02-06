cmake_minimum_required(VERSION 2.8)
project(udev-wait)

add_definitions(-std=c++11)

set(SOURCE src/udev-wait.cpp)

add_executable(udev-wait ${SOURCE})
target_link_libraries(udev-wait udev)

add_custom_target(format clang-format -i ${SOURCE}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

install(TARGETS udev-wait RUNTIME DESTINATION bin)
