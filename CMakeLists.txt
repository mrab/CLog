cmake_minimum_required (VERSION 3.13)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

project (CLog)

set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "ON")

set(CLOG_ENABLE_COVERAGE OFF CACHE BOOL "OFF")
set(CLOG_BUILD_SAMPLES ON CACHE BOOL "ON")
set(CLOG_TEST ON CACHE BOOL "ON")

message("CLOG Build options:")
message(" - CLOG_ENABLE_COVERAGE: ${CLOG_ENABLE_COVERAGE}")
message(" - CLOG_BUILD_SAMPLES:   ${CLOG_BUILD_SAMPLES}")
message(" - CLOG_TEST:            ${CLOG_TEST}")

if(CLOG_ENABLE_COVERAGE)
  include(CodeCoverage)

  APPEND_COVERAGE_COMPILER_FLAGS()
endif()

add_compile_options(-Wall -Werror -Wpedantic -Wextra -Wno-gnu-zero-variadic-macro-arguments)

if(CLOG_TEST)
  # Download and unpack googletest at configure time
  configure_file(cmake/CMakeLists.txt.in googletest-download/CMakeLists.txt)
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
  if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
  endif()

  execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
  if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
  endif()

  # Prevent overriding the parent project's compiler/linker
  # settings on Windows
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  # Add googletest directly to our build. This defines
  # the gtest and gtest_main targets.
  add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                  ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                  EXCLUDE_FROM_ALL)
endif()

add_library(CLog 
  src/clog.c
)

target_include_directories(CLog 
  PUBLIC
  include
)
 
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
    COMMAND ${CMAKE_CTEST_COMMAND} -R "^CLogTestColor$" --output-on-failures
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
    SETUP_TARGET_FOR_COVERAGE_GCOVR_HTML(
        NAME CLogTestColor_coverage
        EXECUTABLE CLogTestColor
        DEPENDENCIES CLogTestColor
    )
  endif()

  if(CLOG_BUILD_SAMPLES)
    SETUP_TARGET_FOR_COVERAGE_GCOVR_HTML(
      NAME CLogSample_coverage
      EXECUTABLE CLogSample
      DEPENDENCIES CLogSample
    )
  endif()
endif()