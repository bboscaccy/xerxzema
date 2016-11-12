#pragma once
#include <string>
#include <vector>
#include "Type.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"

namespace xerxzema
{

class Register;

#define DEF_INST(x) class x : public Instruction { \
	std::vector<Type*> input_types();  \
	std::vector<Type*> output_types(); };

#define DEF_INST_VAR(x) class x : public Instruction { \
	std::vector<Type*> input_types(); \
	std::vector<Type*> output_types(); \
	inline bool has_variadic_output() { return true; } };\


class Instruction
{
public:
	Instruction();
	virtual std::vector<Type*> input_types();
	virtual std::vector<Type*> output_types();
	virtual bool has_variadic_output();

	void input(Register* reg);
	void dependent(Register* reg);
	void sample(Register* reg);
	inline std::vector<Register*>& inputs()
	{
		return _inputs;
	}

	inline void output(Register* reg)
	{
		_outputs.push_back(reg);
	}

	inline std::vector<Register*>& outputs()
	{
		return _outputs;
	}

	inline uint32_t offset() { return _offset; }
	inline void offset(uint32_t o)
	{
		_offset = o;
	}

	virtual void generate_check(llvm::LLVMContext& context,
								llvm::IRBuilder<> &builder,
								llvm::Type* state_type,
								llvm::Value* state,
								llvm::BasicBlock* check_block,
								llvm::BasicBlock* op_block,
								llvm::BasicBlock* next_block);

	virtual void generate_operation(llvm::LLVMContext& context,
									llvm::IRBuilder<> &builder,
									llvm::Type* state_type,
									llvm::Value* state);

	virtual void generate_prolouge(llvm::LLVMContext& context,
								   llvm::IRBuilder<> &builder,
								   llvm::Type* state_type,
								   llvm::Value* state,
								   llvm::BasicBlock* next_block);

protected:
	std::vector<Register*> _inputs;
	std::vector<Register*> _outputs;
	std::vector<Register*> _deps;
	uint32_t _offset;
	uint16_t mask;
};

DEF_INST(Add)

};
