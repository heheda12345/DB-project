cmake_minimum_required(VERSION 3.10)
project(DB.out)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g -std=c++11")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -g -std=c++11")

add_subdirectory(parser)
add_subdirectory(ql)
add_subdirectory(sm)
add_subdirectory(ix)
add_subdirectory(rm)
add_subdirectory(pf)
add_subdirectory(utils)

# add_executable(rm_test.out rm_test.cc)
# target_link_libraries(rm_test.out rm pf utils)

add_executable(ix_test.out ix_test.cc)
target_link_libraries(ix_test.out ix rm pf utils)

add_compile_options("-ly")
add_compile_options("-ll")

add_executable(main.out main.cc)
target_link_libraries(main.out parser ql sm ix rm pf utils)