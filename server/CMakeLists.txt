file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  ${md2_SOURCE_DIR}/server/*.h
  ${md2_SOURCE_DIR}/server/*.cc
)

add_library(libmd2server STATIC ${SRC_FILES})

target_include_directories(libmd2server PUBLIC .)
target_compile_features(libmd2server PRIVATE cxx_std_17)

target_link_libraries(libmd2server PUBLIC 
  libmd2 
  fmt::fmt 
  zmq
  nlohmann_json::nlohmann_json
)
