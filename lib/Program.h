#pragma once
#include <map>
#include <string>
#include <memory>
#include "Register.h"
#include "Instruction.h"

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
	void instruction(std::unique_ptr<Instruction>&& inst);
	Register* reg(const std::string& name);
	void code_gen(llvm::Module* module, llvm::LLVMContext& context);

	llvm::FunctionType* function_type(llvm::LLVMContext& context);

private:
	void allocate_registers(llvm::LLVMContext& context, llvm::IRBuilder<>& builder,
							llvm::Value* state);
	std::map<std::string, std::unique_ptr<Register>> registers;
	std::vector<Register*> inputs;
	std::vector<Register*> outputs;
	std::vector<Register*> locals;
	std::vector<std::unique_ptr<Instruction>> instructions;
	std::string _name;
	Namespace* parent;
	llvm::Type* state_type;
	llvm::Function* init_function;
	llvm::Function* exit_function;
	llvm::Function* frame_function;
	llvm::Function* driver_function;
};


};
