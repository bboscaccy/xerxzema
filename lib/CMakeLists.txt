llvm_map_components_to_libnames(llvm_libs x86 mcjit executionengine target native)
add_library(xerxzema SHARED
  Type.cpp
  Store.cpp
  )
target_link_libraries(xerxzema ${llvm_libs})
