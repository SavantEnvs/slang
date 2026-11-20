# Mayhem fuzz build for slang: builds the slang library + CLI tools from the repo root (as a
# subproject, so every upstream object gets the sanitizer/coverage flags from CMAKE_CXX_FLAGS)
# and one libFuzzer binary + one run-once "-standalone" reproducer per harness.
cmake_minimum_required(VERSION 3.28)
project(slang_mayhem LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Same as upstream's SLANG_FUZZ_TARGET: turn slang's internal assertions on everywhere
# (it changes struct layout, so it must be global), so ICEs surface as crashes.
add_compile_definitions(SLANG_ASSERT_ENABLED)

set(SLANG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." CACHE PATH "slang source root")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_subdirectory(${SLANG_ROOT} slang)

set(FUZZ_ENGINE "-fsanitize=fuzzer" CACHE STRING "libFuzzer link flag")
set(STANDALONE_MAIN "" CACHE FILEPATH "LLVM StandaloneFuzzTargetMain.c")
add_library(standalone_main OBJECT ${STANDALONE_MAIN})
set(LSAN_OFF_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../lsan_off.cc" CACHE FILEPATH "__lsan_is_turned_off TU")
add_library(lsan_off OBJECT ${LSAN_OFF_SRC})

function(slang_fuzzer name)
  set(srcs ${ARGN})
  add_executable(${name} ${srcs} $<TARGET_OBJECTS:lsan_off>)
  target_link_libraries(${name} PRIVATE slang::slang fmt::fmt)
  target_link_options(${name} PRIVATE ${FUZZ_ENGINE})
  add_executable(${name}-standalone ${srcs} $<TARGET_OBJECTS:lsan_off>
                 $<TARGET_OBJECTS:standalone_main>)
  target_link_libraries(${name}-standalone PRIVATE slang::slang fmt::fmt)
  set_target_properties(${name} ${name}-standalone PROPERTIES
                        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/fuzzers)
endfunction()

# Upstream's own libFuzzer entry point (tools/driver/slang_main.cpp under FUZZ_TARGET).
slang_fuzzer(fuzz_slang_driver ${SLANG_ROOT}/tools/driver/slang_main.cpp)
target_compile_definitions(fuzz_slang_driver PRIVATE FUZZ_TARGET)
target_compile_definitions(fuzz_slang_driver-standalone PRIVATE FUZZ_TARGET)

foreach(h compile_analyze script_eval parse_text preprocess ast_json libmap strToUpper svint
        command_file)
  slang_fuzzer(fuzz_${h} ${CMAKE_CURRENT_SOURCE_DIR}/fuzz_${h}.cpp)
endforeach()

# Harnesses over the slang-reflect and slang-tidy tool libraries.
slang_fuzzer(fuzz_reflect ${CMAKE_CURRENT_SOURCE_DIR}/fuzz_reflect.cpp)
target_link_libraries(fuzz_reflect PRIVATE slang_reflect_obj_lib)
target_link_libraries(fuzz_reflect-standalone PRIVATE slang_reflect_obj_lib)
slang_fuzzer(fuzz_tidy ${CMAKE_CURRENT_SOURCE_DIR}/fuzz_tidy.cpp)
target_link_libraries(fuzz_tidy PRIVATE slang_tidy_obj_lib)
target_link_libraries(fuzz_tidy-standalone PRIVATE slang_tidy_obj_lib)

# The `slang` and `slang-tidy` CLIs are file-input Mayhem targets (`slang`, `waivers`,
# `tidy-config`): give them the LeakSanitizer-off hook too.
target_link_libraries(slang_driver PRIVATE lsan_off)
target_link_libraries(slang_tidy PRIVATE lsan_off)
