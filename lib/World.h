#pragma once
#include <string>
#include <map>
#include <memory>
#include "Namespace.h"
#include "Jit.h"
#include "Scheduler.h"
#include "llvm/Support/DynamicLibrary.h"

namespace xerxzema
{
class ExternalDefinition
{
public:
	ExternalDefinition(const std::string& symbol_name, const std::vector<Type*> types,
					   Type* result_type, const std::string& lib_name = "",
					   void* address = nullptr, bool var_arg = false);

	llvm::Function* get_call(llvm::Module* module, llvm::LLVMContext& context);
	llvm::GlobalVariable* get_variable(llvm::Module* module, llvm::LLVMContext& context);

private:
	std::string symbol_name;
	llvm::sys::DynamicLibrary lib;
	std::vector<Type*> types;
	Type* result_type;
	bool var_arg;
	void* address;
};

class World
{
public:
	World();
	Namespace* get_namespace(const std::string& name);
	std::vector<std::string> namespace_list() const;
	Jit* jit();
	inline Scheduler* scheduler() { return _scheduler.get(); }
private:
	void create_core_namespace();
	std::map<std::string, std::unique_ptr<Namespace>> namespaces;
	std::map<std::string, std::unique_ptr<ExternalDefinition>> externals;
	std::unique_ptr<Scheduler> _scheduler;
	std::unique_ptr<Jit> jit_instance;
};
};
