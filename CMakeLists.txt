cmake_minimum_required(VERSION 3.20)

project(mini_compiler VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(mini_ir STATIC
  lib/Version.cpp
  lib/ir/Type.cpp
  lib/ir/Value.cpp
  lib/ir/Operation.cpp
  lib/ir/Block.cpp
  lib/ir/Region.cpp
  lib/ir/FuncOp.cpp
  lib/ir/ModuleOp.cpp
  lib/ir/Walk.cpp
  lib/ir/Parser.cpp
  lib/ir/Printer.cpp
  lib/ir/Verify.cpp
  lib/ops/MiniAddOp.cpp
  lib/ops/MiniReluOp.cpp
  lib/ops/MiniMatmulOp.cpp
  lib/pass/PassManager.cpp
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

include(CTest)
if(BUILD_TESTING)
  add_executable(ir-use-def-test tests/ir_use_def.cpp)
  target_link_libraries(ir-use-def-test PRIVATE mini_ir)
  add_test(NAME ir-use-def COMMAND ir-use-def-test)

  add_executable(ir-containers-test tests/ir_containers.cpp)
  target_link_libraries(ir-containers-test PRIVATE mini_ir)
  add_test(NAME ir-containers COMMAND ir-containers-test)

  add_executable(ir-rewrite-test tests/ir_rewrite.cpp)
  target_link_libraries(ir-rewrite-test PRIVATE mini_ir)
  add_test(NAME ir-rewrite COMMAND ir-rewrite-test)

  add_executable(ir-text-test tests/ir_text.cpp)
  target_link_libraries(ir-text-test PRIVATE mini_ir)
  add_test(NAME ir-text COMMAND ir-text-test)

  add_executable(ir-verify-test tests/ir_verify.cpp)
  target_link_libraries(ir-verify-test PRIVATE mini_ir)
  add_test(NAME ir-verify COMMAND ir-verify-test)

  add_executable(ir-func-verify-test tests/ir_func_verify.cpp)
  target_link_libraries(ir-func-verify-test PRIVATE mini_ir)
  add_test(NAME ir-func-verify COMMAND ir-func-verify-test)

  add_executable(ir-module-verify-test tests/ir_module_verify.cpp)
  target_link_libraries(ir-module-verify-test PRIVATE mini_ir)
  add_test(NAME ir-module-verify COMMAND ir-module-verify-test)

  add_executable(pass-manager-test tests/pass_manager.cpp)
  target_link_libraries(pass-manager-test PRIVATE mini_ir)
  add_test(NAME pass-manager COMMAND pass-manager-test)
endif()
