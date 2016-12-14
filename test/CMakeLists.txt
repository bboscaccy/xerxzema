add_executable(unittest
  NamespaceTests.cpp
  JitTests.cpp
  ProgramTests.cpp
  LexerTests.cpp
  ParserTests.cpp
  SemanticTests.cpp
  DiagnosticTests.cpp
  SchedulerTests.cpp
  )

include_directories(../lib)
target_link_libraries(unittest gtest gtest_main pthread xerxzema)
add_test(unittests ${CMAKE_CURRENT_BINARY_DIR}/unittest)
