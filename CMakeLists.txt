cmake_minimum_required(VERSION "3.0.0")
project("quad-world" VERSION "0.1.0")

add_executable("${PROJECT_NAME}" "src/main.cpp")

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME "${PROJECT_NAME}")
set(CPACK_PROJECT_VERSION "${PROJECT_VERSION}")
include(CPack)