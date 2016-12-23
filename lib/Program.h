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
class Expression;

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
	std::vector<RegisterData> dependencies;
	bool solved;
	Expression* source;
};

//TODO figure out a decent way of doing diffs from an old version to a new version
//then start updating the global symbol resolver in the runtime...
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
					 const std::vector<RegisterData>& outputs,
					 const std::vector<RegisterData>& dependencies = {},
					 Expression* source = nullptr);
	RegisterData constant(double literal);
	RegisterData constant_int(int64_t literal);
	Register* reg(const std::string& name);
	inline RegisterData reg_data(const std::string& name, bool sample=false)
	{
		return RegisterData({reg(name), sample});
	}

	inline RegisterData temp_reg()
	{
		return reg_data("temp." + std::to_string(locals.size() + 1));
	}

	void code_gen(llvm::Module* module, llvm::LLVMContext& context);

	llvm::FunctionType* function_type(llvm::LLVMContext& context);
	inline llvm::Value* activation_counter_value() { return activation_counter; }
	inline llvm::Function* function_value() { return function; }
	inline llvm::Module* current_module() { return _current_module; }
	inline llvm::Type* state_type_value() { return state_type; }
	inline llvm::Value* current_state() { return program_state; }
	inline Namespace* name_space() { return parent; }

	inline std::string program_name() { return root_name; }
	inline std::vector<Register*>& input_registers() { return inputs; }
	inline std::vector<Register*>& output_registers() { return outputs; }

	inline const std::vector<std::unique_ptr<Instruction>>& instruction_listing()
	{
		return instructions;
	}

	inline const std::map<std::string, std::unique_ptr<Register>>& register_listing()
	{
		return registers;
	}

	std::string symbol_name();
	llvm::Value* create_closure(Register* reg, bool reinvoke,
								llvm::LLVMContext& context, llvm::Module* module);
	inline bool is_valid() const { return valid; }
	inline llvm::GlobalVariable* get_call_site() { return call_site; }
	llvm::Function* create_declaration(llvm::Module* module, llvm::LLVMContext& context);

private:
	bool check_instruction(const std::string& name,
						   const std::vector<RegisterData>& inputs,
						   const std::vector<RegisterData>& outputs,
						   const std::vector<RegisterData>& dependencies,
						   Expression* source);
	void allocate_registers(llvm::LLVMContext& context, llvm::IRBuilder<>& builder,
							llvm::Function* fn);
	void generate_exit_block(llvm::LLVMContext& context, llvm::IRBuilder<>& builder);
	llvm::Function* trampoline_gen(llvm::Module* module, llvm::LLVMContext& context,
								   llvm::GlobalVariable* target_call, const std::string& call_name);
	void transform_stub_gen(llvm::Module* module, llvm::LLVMContext& context);
	void transform_gen(llvm::Module* module, llvm::LLVMContext& context);
	llvm::BasicBlock* generate_entry_block(llvm::LLVMContext& context, llvm::IRBuilder<>& builder);

	std::map<std::string, std::unique_ptr<Register>> registers;
	std::vector<Register*> inputs;
	std::vector<Register*> outputs;
	std::vector<Register*> locals;
	std::vector<std::unique_ptr<Instruction>> instructions;
	std::string root_name;
	Namespace* parent;
	llvm::Type* state_type;
	llvm::Value* activation_counter;
	llvm::Function* function;
	llvm::Function* transformer;
	llvm::Function* trampoline_entry;
	llvm::Module* _current_module;
	llvm::GlobalVariable* call_site;
	llvm::GlobalVariable* version_number;
	llvm::GlobalVariable* transform_site;
	std::vector<std::unique_ptr<DeferredInstruction>> deferred;
	bool is_trivial;
	llvm::Value* program_state;
	bool valid;
};


};
