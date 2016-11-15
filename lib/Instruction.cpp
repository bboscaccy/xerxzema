#include "Instruction.h"
#include "Register.h"

namespace xerxzema
{

Instruction::Instruction()
{

}

void Instruction::input(xerxzema::Register *reg)
{
	_inputs.push_back(reg);
	dependent(reg);
}

void Instruction::dependent(xerxzema::Register *reg)
{
	reg->activation(this, 1 << _deps.size());
	_deps.push_back(reg);
}

void Instruction::sample(xerxzema::Register *reg)
{
	_inputs.push_back(reg);
}

void Instruction::generate_check(llvm::LLVMContext& context,
								 llvm::IRBuilder<> &builder,
								 llvm::Type* state_type,
								 llvm::Value* state,
								 llvm::BasicBlock* check_block,
								 llvm::BasicBlock* op_block,
								 llvm::BasicBlock* next_block)
{
	builder.SetInsertPoint(check_block);
	auto mask_value = builder.CreateLoad(_value);
	auto comp_value = builder.CreateICmp(llvm::CmpInst::Predicate::ICMP_EQ,
										 mask_value,
										 llvm::ConstantInt::get(context, llvm::APInt(16, mask)));
	builder.CreateCondBr(comp_value, op_block, next_block);
}

void Instruction::generate_operation(llvm::LLVMContext &context,
									 llvm::IRBuilder<> &builder,
									 llvm::Type *state_type,
									 llvm::Value *state)
{
	builder.CreateAlloca(llvm::Type::getVoidTy(context), nullptr, "nop");

}

void Instruction::generate_prolouge(llvm::LLVMContext &context,
									llvm::IRBuilder<> &builder,
									llvm::Type *state_type,
									llvm::Value *state,
									llvm::BasicBlock *next_block)
{
	auto p = builder.CreateLoad(state);
	auto i = builder.CreateAdd(p, llvm::ConstantInt::get(context, llvm::APInt(64,1)));
	builder.CreateStore(i, state);

	builder.CreateStore(_value, llvm::ConstantInt::get(context, llvm::APInt(16, 0)));
	for(auto& r:_outputs)
	{
		r->do_activations(context, builder, state_type, state);
	}
	builder.CreateBr(next_block);
}

ValueReal::ValueReal(double v):value(v)	{}
void ValueReal::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
								   llvm::Type *state_type, llvm::Value *state)
{
	auto const_value = llvm::ConstantFP::get(llvm::Type::getDoubleTy(context), value);
	auto target = _outputs[0]->fetch_value_raw(context, builder, state_type, state);
	auto p = builder.CreateStore(target, const_value);
}

};
