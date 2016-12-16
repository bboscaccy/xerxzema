#pragma once
#include <string>
#include <vector>
#include "Type.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"

namespace xerxzema
{

#define DECL_INST(X, N) class X : public Instruction {					\
	void generate_operation(llvm::LLVMContext& context,	llvm::IRBuilder<> &builder, \
							Program* program);\
	inline std::string name() { return N; } };

class Register;
class Program;
class Instruction
{
public:
	Instruction();
	virtual ~Instruction() = default;
	void input(Register* reg);
	void dependent(Register* reg);
	void sample(Register* reg);
	void oneshot_dependent(Register* reg);
	virtual inline std::string name() { return "undef"; }
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

	virtual void validate_mask();

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

	inline std::string name() { return "value_real";}
private:
	double value;
};

class ValueInt : public Instruction
{
public:
	ValueInt(int64_t v);
	void generate_operation(llvm::LLVMContext& context,	llvm::IRBuilder<> &builder,
							Program* program);
	inline std::string name() { return "value_int";}
private:
	int64_t value;
};

class Counter : public Instruction
{
public:
	inline bool is_ugen() { return true; }
	inline std::string name() { return "counter";}

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
	inline std::string name() { return "delay";}
};

//input[0] is a bool, input[1] is any, output[0] == input[1]
//when i[0] == True, input[0] is copied to output[0]
//otherwise output[0] remains unchanges.
//output[0] is fired regardless
class When : public Instruction
{
public:
	void generate_operation(llvm::LLVMContext& context,
							llvm::IRBuilder<> &builder,
							Program* program);
	void generate_prolouge(llvm::LLVMContext& context,
						   llvm::IRBuilder<> &builder,
						   Program* program,
						   llvm::BasicBlock* next_block);
	inline std::string name() { return "when";}
};

class Cond : public Instruction
{
public:
	void generate_operation(llvm::LLVMContext& context,
							llvm::IRBuilder<> &builder,
							Program* program);
	void generate_prolouge(llvm::LLVMContext& context,
						   llvm::IRBuilder<> &builder,
						   Program* program,
						   llvm::BasicBlock* next_block);
	inline std::string name() { return "cond";}
};

class Bang : public Instruction
{
public:
	void generate_operation(llvm::LLVMContext& context,
							llvm::IRBuilder<> &builder,
							Program* program);
	inline std::string name() { return "bang";}
};

class Trace : public Instruction
{
public:
	void generate_operation(llvm::LLVMContext& context,
							llvm::IRBuilder<> &builder,
							Program* program);
	void generate_prolouge(llvm::LLVMContext& context,
						   llvm::IRBuilder<> &builder,
						   Program* program,
						   llvm::BasicBlock* next_block);
	inline std::string name() { return "trace";}
};

class Schedule : public Instruction
{
public:
	void generate_operation(llvm::LLVMContext& context,
							llvm::IRBuilder<> &builder,
							Program* program);
	void generate_prolouge(llvm::LLVMContext& context,
						   llvm::IRBuilder<> &builder,
						   Program* program,
						   llvm::BasicBlock* next_block);
	inline std::string name() { return "schedule";}
};

class Merge : public Instruction
{
public:
	void generate_check(llvm::LLVMContext& context,
						llvm::IRBuilder<> &builder,
						Program* program,
						llvm::BasicBlock* check_block,
						llvm::BasicBlock* op_block,
						llvm::BasicBlock* next_block);

	void generate_operation(llvm::LLVMContext& context,
							llvm::IRBuilder<> &builder,
							Program* program);
	void generate_prolouge(llvm::LLVMContext& context,
						   llvm::IRBuilder<> &builder,
						   Program* program,
						   llvm::BasicBlock* next_block);
	inline std::string name() { return "merge";}
};


DECL_INST(AddReal, "add")
DECL_INST(SubReal, "sub")
DECL_INST(MulReal, "mul")
DECL_INST(DivReal, "div")
DECL_INST(PowReal, "pow")
DECL_INST(EqReal, "eq")
DECL_INST(NeReal, "ne")
DECL_INST(LtReal, "lt")
DECL_INST(LeReal, "le")
DECL_INST(GtReal, "gt")
DECL_INST(GeReal, "ge")

};
