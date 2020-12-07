# Works with 3.11 and tested through 3.18
cmake_minimum_required(VERSION 3.11...3.18)

# Project name and a few useful settings. Other commands can pick up the results
project(
  md2
  VERSION 0.1
  DESCRIPTION "Second generation of the Markdown parser."
  LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Formatting library
FetchContent_Declare(
  fmtlib
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 7.0.3)
FetchContent_MakeAvailable(fmtlib)
# Adds fmt::fmt

# JSON library
FetchContent_Declare(
  json 
  GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
  GIT_TAG v3.9.1)
FetchContent_MakeAvailable(json)

# The compiled library code is here
add_subdirectory(src)

# The executable code is here
add_subdirectory(app)

option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    enable_testing()
    include(GoogleTest)
    add_subdirectory(tests)
    add_test(md2test md2test)
endif()
