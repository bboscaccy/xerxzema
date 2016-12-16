llvm_map_components_to_libnames(llvm_libs x86 mcjit executionengine target native )
add_library(xerxzema SHARED
  Type.cpp
  Register.cpp
  Program.cpp
  World.cpp
  Namespace.cpp
  Jit.cpp
  Instruction.cpp
  Lexer.cpp
  Ast.cpp
  Parser.cpp
  Semantic.cpp
  Diagnostics.cpp
  Scheduler.cpp
  RT.cpp
  Session.cpp
  Transformer.cpp
  )
target_link_libraries(xerxzema ${llvm_libs})
