add_executable(unittest
  NamespaceTests.cpp
  )

include_directories(../lib)
target_link_libraries(unittest gtest gtest_main pthread xerxzema)
add_test(unittests ${CMAKE_CURRENT_BINARY_DIR}/unittest)
