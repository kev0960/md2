include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.10.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
set(BUILD_GTEST ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# Add test cpp file
add_executable(md2test driver_test.cc parser_test.cc html_test.cc metadata_test.cc)

# Link test executable against gtest & gtest_main
target_link_libraries(md2test libmd2 gtest gmock gtest_main fmt::fmt)
