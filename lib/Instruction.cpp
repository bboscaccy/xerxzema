#include "Instruction.h"
#include "Register.h"
#include "Program.h"

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
	mask |= (1 << _deps.size());
	_deps.push_back(reg);

}

void Instruction::sample(xerxzema::Register *reg)
{
	_inputs.push_back(reg);
	reg->activation(this, 1 << _deps.size());
	reset_mask |= (1 << _deps.size());
	_deps.push_back(reg);
}

void Instruction::generate_check(llvm::LLVMContext& context,
								 llvm::IRBuilder<> &builder,
								 Program* program,
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
									 Program* program)
{
	builder.CreateAlloca(llvm::Type::getVoidTy(context), nullptr, "nop");

}

void Instruction::generate_prolouge(llvm::LLVMContext &context,
									llvm::IRBuilder<> &builder,
									Program* program,
									llvm::BasicBlock *next_block)
{
	auto p = builder.CreateLoad(program->activation_counter_value());
	auto i = builder.CreateAdd(p, llvm::ConstantInt::get(context, llvm::APInt(64,1)));
	builder.CreateStore(i, program->activation_counter_value());

	builder.CreateStore(_value, llvm::ConstantInt::get(context, llvm::APInt(16, reset_mask)));
	for(auto& r:_outputs)
	{
		r->do_activations(context, builder);
	}
	builder.CreateBr(next_block);
}

ValueReal::ValueReal(double v):value(v)	{}
void ValueReal::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
								   Program* program)
{
	auto const_value = llvm::ConstantFP::get(llvm::Type::getDoubleTy(context), value);
	auto target = _outputs[0]->fetch_value_raw(context, builder);
	auto p = builder.CreateStore(target, const_value);
}

};
