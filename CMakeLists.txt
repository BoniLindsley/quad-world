cmake_minimum_required(VERSION "3.0.0")
project("quad-world" VERSION "0.1.0")

add_executable("${PROJECT_NAME}" "src/main.cpp")

find_package(OpenGL REQUIRED)
target_link_libraries("${PROJECT_NAME}" PRIVATE OpenGL::GL)

find_package(SDL2 REQUIRED)
target_link_libraries("${PROJECT_NAME}" PRIVATE SDL2::SDL2 SDL2::SDL2main)

find_package(imgui REQUIRED)
target_link_libraries("${PROJECT_NAME}" PRIVATE imgui::imgui)

include(CTest)
enable_testing()

include(CPack)