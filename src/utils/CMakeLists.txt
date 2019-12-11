project(utils)
add_compile_options("-g")
add_compile_options("-std=c++11")

aux_source_directory(. DIR_LIB_SRCS)
add_library (utils ${DIR_LIB_SRCS})