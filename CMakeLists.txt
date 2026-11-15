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
  lib/ops/MiniDpsAddOp.cpp
  lib/ops/MiniDpsReluOp.cpp
  lib/ops/SideEffectInterface.cpp
  lib/pass/PassManager.cpp
  lib/pass/CanonicalizePass.cpp
  lib/pass/CsePass.cpp
  lib/analysis/UseCountAnalysis.cpp
  lib/analysis/AnalysisManager.cpp
  lib/interpreter/Interpreter.cpp
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

  add_executable(use-count-analysis-test tests/use_count_analysis.cpp)
  target_link_libraries(use-count-analysis-test PRIVATE mini_ir)
  add_test(NAME use-count-analysis COMMAND use-count-analysis-test)

  add_executable(analysis-cache-test tests/analysis_cache.cpp)
  target_link_libraries(analysis-cache-test PRIVATE mini_ir)
  add_test(NAME analysis-cache COMMAND analysis-cache-test)

  add_executable(structured-ops-test tests/structured_ops.cpp)
  target_link_libraries(structured-ops-test PRIVATE mini_ir)
  add_test(NAME structured-ops COMMAND structured-ops-test)

  add_executable(dps-ops-test tests/dps_ops.cpp)
  target_link_libraries(dps-ops-test PRIVATE mini_ir)
  add_test(NAME dps-ops COMMAND dps-ops-test)

  add_executable(interpreter-test tests/interpreter.cpp)
  target_link_libraries(interpreter-test PRIVATE mini_ir)
  add_test(NAME interpreter COMMAND interpreter-test)

  add_executable(canonicalize-test tests/canonicalize.cpp)
  target_link_libraries(canonicalize-test PRIVATE mini_ir)
  add_test(NAME canonicalize COMMAND canonicalize-test)

  add_executable(cse-test tests/cse.cpp)
  target_link_libraries(cse-test PRIVATE mini_ir)
  add_test(NAME cse COMMAND cse-test)
endif()
