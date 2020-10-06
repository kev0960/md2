file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  ${md2_SOURCE_DIR}/src/*.h
  ${md2_SOURCE_DIR}/src/*.cc
)

add_library(libmd2 STATIC ${SRC_FILES})
target_include_directories(libmd2 PUBLIC .)
target_compile_features(libmd2 PUBLIC cxx_std_17)

