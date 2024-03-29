cmake_minimum_required(VERSION 3.22)
project(quad-world VERSION 0.1.0)

find_package(imgui REQUIRED)

set(TARGET_NAME ${PROJECT_NAME})
add_executable(
  ${TARGET_NAME}
  src/main.cpp
  # "${imgui_PACKAGE_FOLDER_RELEASE}/res/bindings/imgui_impl_sdl2.cpp"
  # "${imgui_PACKAGE_FOLDER_RELEASE}/res/bindings/imgui_impl_sdlrenderer2.cpp"
)
set_target_properties(
  ${TARGET_NAME} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF
)

find_package(Boost REQUIRED)

find_package(OpenGL REQUIRED)
target_link_libraries(${TARGET_NAME} PRIVATE OpenGL::GL)

find_package(SDL2 REQUIRED)
target_link_libraries(${TARGET_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main)

target_link_libraries(${TARGET_NAME} PRIVATE imgui::imgui)
target_include_directories(
  ${TARGET_NAME} PUBLIC ${imgui_PACKAGE_FOLDER_RELEASE}/res/bindings
)

include(CTest)
if(BUILD_TESTING)
  set(TARGET_NAME ${PROJECT_NAME}-test)
  add_executable(
    ${TARGET_NAME}
    tests/test_boni/test_memory.cpp tests/test_boni/test_type_traits.cpp
  )
  set_target_properties(
    ${TARGET_NAME} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF
  )

  target_include_directories(${TARGET_NAME} PRIVATE src)

  find_package(Catch2 REQUIRED)
  target_link_libraries(${TARGET_NAME} Catch2::Catch2WithMain)

  add_test(NAME "${TARGET_NAME}" COMMAND "${TARGET_NAME}")

  include(Catch)
  catch_discover_tests(${TARGET_NAME})
endif()

find_package(Doxygen)
doxygen_add_docs(doc)

include(CPack)
