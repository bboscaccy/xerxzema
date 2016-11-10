llvm_map_components_to_libnames(llvm_libs x86 mcjit executionengine target native
  aarch64 aarch64asmparser aarch64asmprinter aarch64codegen aarch64desc aarch64disassembler aarch64info aarch64utils
  amdgpu amdgpuasmparser amdgpuasmprinter amdgpucodegen amdgpudesc amdgpudisassembler amdgpuinfo amdgpuutils
  arm armasmparser armasmprinter armcodegen armdesc armdisassembler arminfo
  bpf bpfasmprinter bpfcodegen bpfdesc bpfinfo
  hexagon hexagonasmparser hexagoncodegen hexagondesc hexagondisassembler hexagoninfo
  mips mipsasmparser mipsasmprinter mipscodegen mipsdesc mipsdisassembler mipsinfo mirparser
  msp430 msp430asmprinter msp430codegen msp430desc msp430info
  native nativecodegen
  nvptx nvptxasmprinter nvptxcodegen nvptxdesc nvptxinfo
  powerpc powerpcasmparser powerpcasmprinter powerpccodegen powerpcdesc powerpcdisassembler powerpcinfo
  sparc sparcasmparser sparcasmprinter sparccodegen sparcdesc sparcdisassembler sparcinfo
  systemz systemzasmparser systemzasmprinter systemzcodegen systemzdesc systemzdisassembler systemzinfo
  x86 x86asmparser x86asmprinter x86codegen x86desc x86disassembler x86info x86utils
  xcore xcoreasmprinter xcorecodegen xcoredesc xcoredisassembler xcoreinfo  )
add_library(xerxzema SHARED
  Type.cpp
  Register.cpp
  Program.cpp
  World.cpp
  Namespace.cpp
  )
target_link_libraries(xerxzema ${llvm_libs})
