#include "Instruction.h"
#include "Register.h"

namespace xerxzema
{

Instruction::Instruction()
{

}

bool Instruction::has_variadic_output()
{
	return false;
}

std::vector<Type*> Instruction::input_types()
{
	return std::vector<Type*>();
}

std::vector<Type*> Instruction::output_types()
{
	return std::vector<Type*>();
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
	auto mask_ptr = builder.CreateStructGEP(state_type, state, _offset);
	auto mask_value = builder.CreateLoad(mask_ptr);
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
	builder.CreateAdd(llvm::ConstantInt::get(context, llvm::APInt(1, 0)),
					  llvm::ConstantInt::get(context, llvm::APInt(1, 0)));

}

void Instruction::generate_prolouge(llvm::LLVMContext &context,
									llvm::IRBuilder<> &builder,
									llvm::Type *state_type,
									llvm::Value *state,
									llvm::BasicBlock *next_block)
{
	for(auto& r:_outputs)
	{
		r->do_activations(context, builder, state_type, state);
	}
	builder.CreateBr(next_block);
}

};
