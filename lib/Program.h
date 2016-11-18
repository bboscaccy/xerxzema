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

struct DeferredInstruction
{
	std::string name;
	std::vector<std::string> inputs;
	std::vector<std::string> outputs;
	bool solved;
};

class Program
{
public:
	Program(Namespace* parent, const std::string& name);
	void add_input(const std::string& name, Type* type);
	void add_output(const std::string& name, Type* type);
	void instruction(std::unique_ptr<Instruction>&& inst);
	void instruction(const std::string& name,
					 const std::vector<std::string>& inputs,
					 const std::vector<std::string>& outputs);
	Register* reg(const std::string& name);
	void code_gen(llvm::Module* module, llvm::LLVMContext& context);

	llvm::FunctionType* function_type(llvm::LLVMContext& context);
	inline llvm::Value* activation_counter_value() { return activation_counter; }
	inline llvm::Function* function_value() { return function; }

private:
	bool check_instruction(const std::string& name,
						   const std::vector<std::string>& inputs,
						   const std::vector<std::string>& outputs);
	void allocate_registers(llvm::LLVMContext& context, llvm::IRBuilder<>& builder,
							llvm::Function* fn);
	void generate_exit_block(llvm::LLVMContext& context, llvm::IRBuilder<>& builder);
	std::map<std::string, std::unique_ptr<Register>> registers;
	std::vector<Register*> inputs;
	std::vector<Register*> outputs;
	std::vector<Register*> locals;
	std::vector<std::unique_ptr<Instruction>> instructions;
	std::string _name;
	Namespace* parent;
	llvm::Type* state_type;
	llvm::Value* activation_counter;
	llvm::Function* function;
	std::vector<std::unique_ptr<DeferredInstruction>> deferred;
};


};
