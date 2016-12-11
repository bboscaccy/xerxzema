#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>

#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

namespace xerxzema
{
class World;
class Namespace;

class Jit
{
public:
	Jit(World* world);
	inline llvm::LLVMContext& context() { return _context; }
	void compile_namespace(Namespace* ns);
	void* get_jitted_function(const std::string& ns, const std::string& name);

	inline void dump_after_codegen() { dump_pre_optimization = true; }
	inline void dump_after_optimization() { dump_post_optimization = true; }
	size_t get_state_size(const std::string& ns, const std::string& name);

private:
	void create_module(Namespace* ns);
	llvm::LLVMContext _context;
	World* _world;
	std::map<std::string, llvm::Module*> modules;
	std::map<std::string, std::unique_ptr<llvm::ExecutionEngine>> engines;
	bool dump_pre_optimization;
	bool dump_post_optimization;
	void* scheduler;
};
};
