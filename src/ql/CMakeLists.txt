project(ql)
add_compile_options("-g")
add_compile_options("-std=c++11")

aux_source_directory(. DIR_LIB_SRCS)
add_library (ql ${DIR_LIB_SRCS})
