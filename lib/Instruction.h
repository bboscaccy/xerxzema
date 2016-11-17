#pragma once
#include <string>
#include <vector>
#include "Type.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"

namespace xerxzema
{

#define DECL_INST(X) class X : public Instruction { \
	void generate_operation(llvm::LLVMContext& context,	llvm::IRBuilder<> &builder, \
							Program* program); };

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

	virtual bool is_ugen();

	virtual llvm::Type* state_type(llvm::LLVMContext& context);

	virtual void generate_read(llvm::LLVMContext& context,
							   llvm::IRBuilder<> &builder,
							   Program* program);

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

	virtual void generate_state_initializer(llvm::LLVMContext& context,
											llvm::IRBuilder<> &builder,
											Program* program);

	inline void value(llvm::Value* val) { _value = val; }
	inline llvm::Value* value() { return _value; }

	inline llvm::Value* state_value() { return _state_value; }
	inline void state_value(llvm::Value* val ) { _state_value = val; }

	inline llvm::Value* eof_value() { return _eof_value; }
	inline void eof_value(llvm::Value* val ) { _eof_value = val; }

protected:
	llvm::Value* _value;
	llvm::Value* _state_value;
	llvm::Value* _eof_value;
	std::vector<Register*> _inputs;
	std::vector<Register*> _outputs;
	std::vector<Register*> _deps;
	uint32_t _offset;
	uint16_t mask;
	uint16_t reset_mask;
	llvm::Type* _state_type;
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

class Counter : public Instruction
{
public:
	inline bool is_ugen() { return true; }

};

class Delay : public Instruction
{
public:
	llvm::Type* state_type(llvm::LLVMContext& context);
	void generate_operation(llvm::LLVMContext& context,
							llvm::IRBuilder<> &builder,
							Program* program);
	void generate_prolouge(llvm::LLVMContext& context,
						   llvm::IRBuilder<> &builder,
						   Program* program,
						   llvm::BasicBlock* next_block);
};

DECL_INST(AddReal)
DECL_INST(SubReal)
DECL_INST(MulReal)
DECL_INST(DivReal)

};
