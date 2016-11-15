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
class Program;
class Instruction
{
public:
	Instruction();
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
								Program* program,
								llvm::BasicBlock* check_block,
								llvm::BasicBlock* op_block,
								llvm::BasicBlock* next_block);

	virtual void generate_operation(llvm::LLVMContext& context,
									llvm::IRBuilder<> &builder,
									Program* program);

	virtual void generate_prolouge(llvm::LLVMContext& context,
								   llvm::IRBuilder<> &builder,
								   Program* program,
								   llvm::BasicBlock* next_block);

	inline void value(llvm::Value* val) { _value = val; }
	inline llvm::Value* value() { return _value; }

protected:
	llvm::Value* _value;
	std::vector<Register*> _inputs;
	std::vector<Register*> _outputs;
	std::vector<Register*> _deps;
	uint32_t _offset;
	uint16_t mask;
};

class ValueReal : public Instruction
{
public:
	ValueReal(double v);
	void generate_operation(llvm::LLVMContext& context,	llvm::IRBuilder<> &builder,
							Program* program);
private:
	double value;
};

};
