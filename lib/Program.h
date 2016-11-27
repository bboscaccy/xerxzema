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

struct RegisterData
{
	Register* reg;
	bool sample;
};

struct DeferredInstruction
{
	std::string name;
	std::vector<RegisterData> inputs;
	std::vector<RegisterData> outputs;
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
	void instruction(const std::string& name,
					 const std::vector<RegisterData>& inputs,
					 const std::vector<RegisterData>& outputs);
	Register* reg(const std::string& name);
	inline RegisterData reg_data(const std::string& name, bool sample=false)
	{
		return RegisterData({reg(name), sample});
	}

	inline RegisterData temp_reg()
	{
		return reg_data("temp@" + std::to_string(locals.size() + 1));
	}

	void code_gen(llvm::Module* module, llvm::LLVMContext& context);

	llvm::FunctionType* function_type(llvm::LLVMContext& context);
	inline llvm::Value* activation_counter_value() { return activation_counter; }
	inline llvm::Function* function_value() { return function; }
	inline llvm::Module* current_module() { return _current_module; }

	inline std::string program_name() { return _name; }
	inline std::vector<Register*> input_registers() { return inputs; }
	inline std::vector<Register*> output_registers() { return outputs; }

	inline const std::vector<std::unique_ptr<Instruction>>& instruction_listing()
	{
		return instructions;
	}

private:
	bool check_instruction(const std::string& name,
						   const std::vector<RegisterData>& inputs,
						   const std::vector<RegisterData>& outputs);
	void allocate_registers(llvm::LLVMContext& context, llvm::IRBuilder<>& builder,
							llvm::Function* fn);
	void generate_exit_block(llvm::LLVMContext& context, llvm::IRBuilder<>& builder);
	llvm::BasicBlock* generate_entry_block(llvm::LLVMContext& context, llvm::IRBuilder<>& builder);
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
	llvm::Module* _current_module;
	std::vector<std::unique_ptr<DeferredInstruction>> deferred;
};


};
