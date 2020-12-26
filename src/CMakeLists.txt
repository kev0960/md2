include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.10.0
)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  ${md2_SOURCE_DIR}/src/*.h
  ${md2_SOURCE_DIR}/src/*.cc
)

add_library(libmd2 STATIC ${SRC_FILES})

target_link_libraries(libmd2
  fmt::fmt
  pthread
  nlohmann_json::nlohmann_json
  zmq
  pqxx
  pq
  ssl
  crypto
  stdc++fs)

target_include_directories(libmd2 PUBLIC .)
target_compile_features(libmd2 PUBLIC cxx_std_17)

if (MSVC)
  target_compile_options(libmd2 PRIVATE /W4 /WX)
else()
  target_compile_options(libmd2 PRIVATE -Wall -Wextra -pedantic -Werror)
endif()
