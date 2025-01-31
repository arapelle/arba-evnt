cmake_minimum_required(VERSION 3.26)

project(basic_cmake_project)

add_executable(${PROJECT_NAME} main.cpp)
add_executable(${PROJECT_NAME}-static main.cpp)
# Find package arba-evnt:
find_package(arba-evnt 0.6.0 CONFIG REQUIRED)
# Use arba-evnt release shared target:
target_link_libraries(${PROJECT_NAME} PRIVATE arba-evnt)
# Use arba-evnt release static target:
target_link_libraries(${PROJECT_NAME}-static PRIVATE arba-evnt-static)
