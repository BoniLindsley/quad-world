cmake_minimum_required(VERSION 3.22)
project(quad-world VERSION 0.1.0)

set(TARGET_NAME ${PROJECT_NAME})
add_executable(${TARGET_NAME} src/main.cpp)
set_target_properties(${TARGET_NAME} PROPERTIES
  CXX_STANDARD 17
  CXX_EXTENSIONS OFF
)

find_package(Boost REQUIRED)

find_package(OpenGL REQUIRED)
target_link_libraries(${TARGET_NAME} PRIVATE OpenGL::GL)

find_package(SDL2 REQUIRED)
target_link_libraries(${TARGET_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main)

find_package(imgui REQUIRED)
target_link_libraries(${TARGET_NAME} PRIVATE imgui::imgui)

set(TARGET_NAME ${PROJECT_NAME}-test)
add_executable(${TARGET_NAME} tests/main.cpp)
set_target_properties(${TARGET_NAME} PROPERTIES
  CXX_STANDARD 17
  CXX_EXTENSIONS OFF
)

target_include_directories(
  ${TARGET_NAME} PRIVATE src
)

find_package(Catch2 REQUIRED)
target_link_libraries(${TARGET_NAME} Catch2::Catch2)

include(CTest)
include(Catch)
catch_discover_tests(${TARGET_NAME})

include(CPack)
