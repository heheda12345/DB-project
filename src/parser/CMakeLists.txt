project (parser)

add_compile_options("-ly")
add_compile_options("-ll")

# bison and flex
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
FLEX_TARGET(MyLex lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.yy.cc)
BISON_TARGET(MyYacc parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.cc DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.h)
ADD_FLEX_BISON_DEPENDENCY(MyLex MyYacc)

add_library(parser ${CMAKE_CURRENT_BINARY_DIR}/lexer.yy.cc ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.cc ${CMAKE_CURRENT_BINARY_DIR}/tree.cc)