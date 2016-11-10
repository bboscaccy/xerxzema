#pragma once
#include <map>
#include <string>
#include <memory>
#include "Register.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"

namespace xerxzema
{
class Namespace;
class Program
{
public:
	Program(Namespace* parent, const std::string& name);
	void add_input(const std::string& name, Type* type);
	void add_output(const std::string& name, Type* type);
	void code_gen(llvm::Module* module, llvm::LLVMContext& context);

private:
	std::map<std::string, std::unique_ptr<Register>> registers;
	std::string _name;
	Namespace* parent;
};


};
