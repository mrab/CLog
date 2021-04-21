cmake_minimum_required (VERSION 3.13)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

project (CLog)


# External dependencies
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/external)


set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "ON")

set(CLOG_ENABLE_COVERAGE OFF CACHE BOOL "OFF")
set(CLOG_BUILD_SAMPLES ON CACHE BOOL "ON")
set(CLOG_TEST ON CACHE BOOL "ON")

message("CLOG Build options:")
message(" - CLOG_ENABLE_COVERAGE: ${CLOG_ENABLE_COVERAGE}")
message(" - CLOG_BUILD_SAMPLES:   ${CLOG_BUILD_SAMPLES}")
message(" - CLOG_TEST:            ${CLOG_TEST}")
message("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")


#add_compile_options(-Wall -Werror -Wpedantic -Wextra -Wno-gnu-zero-variadic-macro-arguments)
add_compile_options(-Wall -Wpedantic -Wextra -Wno-gnu-zero-variadic-macro-arguments)

if(CLOG_TEST)
  enable_testing()
  include(gtest)
endif()

add_library(CLog 
  src/clog.c
)

target_include_directories(CLog 
  PUBLIC
  include
)
 
if(CLOG_ENABLE_COVERAGE)
  include(CodeCoverage)

  target_compile_options(CLog
    PRIVATE
    -fprofile-instr-generate -fcoverage-mapping
  )
  add_link_options(-fprofile-instr-generate -fcoverage-mapping)
endif()


add_library(CLogColor
  src/clog.c
)

target_include_directories(CLogColor
  PUBLIC
  include
)
                                  
target_compile_definitions(
	CLogColor
  PUBLIC -DCLOG_COLOR
)

if(CLOG_ENABLE_COVERAGE)
  target_compile_options(CLogColor
    PRIVATE
    -fprofile-instr-generate -fcoverage-mapping
  )
  add_link_options(-fprofile-instr-generate -fcoverage-mapping)
endif()



if(CLOG_BUILD_SAMPLES)
  add_executable(CLogSample
    examples/sampleApp.c
  )

  target_link_libraries(CLogSample
    CLog
  )

  add_executable(CLogSampleColor
    examples/sampleApp.c
  )

  target_link_libraries(CLogSampleColor
    CLogColor
  )

  add_executable(CLogSampleColorWarning
    examples/sampleApp.c
  )

  target_link_libraries(CLogSampleColorWarning
    CLogColor
  )

  target_compile_definitions(CLogSampleColorWarning
    PUBLIC
    -DCLOG_GLOBAL_MIN_LEVEL=CLOG_MLWRN
  )
endif()

if(CLOG_TEST)
  enable_testing()

  add_executable(CLogTestColor 
    test/clogLevel.cxx
    test/clogMacros.cxx
    test/clogMessage.cxx
    test/clogFormatMessage.cxx
    test/clogLineHeader.cxx
  )

  target_include_directories(CLogTestColor PUBLIC
    test
  )

  target_link_libraries(CLogTestColor
    CLogColor
    pthread
    gtest
    gmock
    gtest_main
  )

  add_test(CLogTestColor CLogTestColor)

  add_custom_command(
      TARGET CLogTestColor
      COMMENT "Run tests"
      POST_BUILD 
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMAND LLVM_PROFILE_FILE=CLogTestColor.profraw ${CMAKE_CTEST_COMMAND}  --output-on-failures -R "^CLogTestColor$"
  )
endif()

set(DOXYGEN_PLANTUML_JAR_PATH /usr/share/plantuml)
set(DOXYGEN_DOT_IMAGE_FORMAT svg)

find_package(Doxygen
             REQUIRED dot)

doxygen_add_docs(clog_doc
  doc
  src
  include
)            

if(CLOG_ENABLE_COVERAGE)
  if(CLOG_TEST)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/CLogTestColor_coverage-llvm-cov)

    add_custom_target(
      CLogTestColor_coverage
      COMMAND llvm-profdata-11 merge -sparse CLogTestColor.profraw -o CLogTestColor.profdata
      COMMAND llvm-cov-11 show CLogTestColor --color --show-region-summary -show-line-counts-or-regions -output-dir=${CMAKE_BINARY_DIR}/CLogTestColor_coverage-llvm-cov -format="html" -instr-profile=CLogTestColor.profdata
      DEPENDS CLogTestColor
    )
  endif()

  if(CLOG_BUILD_SAMPLES)
    setup_target_for_coverage_gcovr_html(
      NAME CLogSample_coverage
      EXECUTABLE CLogSample
      DEPENDENCIES CLogSample
    )
  endif()
endif()