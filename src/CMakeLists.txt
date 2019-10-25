project(DB)

add_subdirectory(rm)
add_subdirectory(pf)
add_compile_options("-g")

add_executable(${CMAKE_PROJECT_NAME} main.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME} rm pf)

add_executable(rm_test rm_test.cc)
target_link_libraries(rm_test rm pf)

