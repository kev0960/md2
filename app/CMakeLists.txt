add_executable(md2 main.cc)
target_compile_features(md2 PRIVATE cxx_std_17)

target_link_libraries(md2 PRIVATE libmd2 fmt::fmt)

