project(pf)
add_compile_options("-g")
aux_source_directory(. DIR_LIB_SRCS)
add_library (pf ${DIR_LIB_SRCS})
