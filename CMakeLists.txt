cmake_minimum_required(VERSION 3.20)

project(mini_compiler VERSION 0.1.0 LANGUAGES CXX)

add_library(mini_ir STATIC
  lib/Version.cpp
)
target_compile_features(mini_ir PUBLIC cxx_std_20)
target_include_directories(mini_ir PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_compile_definitions(mini_ir PRIVATE MINI_COMPILER_VERSION="${PROJECT_VERSION}")

add_executable(mini-opt
  tools/mini-opt/main.cpp
)
target_link_libraries(mini-opt PRIVATE mini_ir)
