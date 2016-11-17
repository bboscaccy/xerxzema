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

bool Instruction::is_ugen()
{
	return false;
}

llvm::Type* Instruction::state_type(llvm::LLVMContext& context)
{
	return nullptr;
}

void Instruction::generate_read(llvm::LLVMContext& context,
								llvm::IRBuilder<> &builder,
								Program* program)
{

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

	builder.CreateStore(llvm::ConstantInt::get(context, llvm::APInt(16, reset_mask)), _value);
	for(auto& r:_outputs)
	{
		r->do_activations(context, builder);
	}
	builder.CreateBr(next_block);
}

void Instruction::generate_state_initializer(llvm::LLVMContext &context,
											 llvm::IRBuilder<> &builder,
											 xerxzema::Program *program)
{
	auto size = llvm::ConstantExpr::getSizeOf(_state_type);
	auto ptr = builder.CreateBitCast(_state_value, llvm::Type::getInt8PtrTy(context));
	builder.CreateMemSet(ptr, llvm::ConstantInt::get(llvm::Type::getInt8Ty(context), 0), size, 0);
}

ValueReal::ValueReal(double v):value(v)	{}
void ValueReal::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
								   Program* program)
{
	auto const_value = llvm::ConstantFP::get(llvm::Type::getDoubleTy(context), value);
	auto target = _outputs[0]->fetch_value_raw(context, builder);
	auto p = builder.CreateStore(const_value, target);
}

void AddReal::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
								 xerxzema::Program *program)
{
	auto lhs = _inputs[0]->fetch_value(context, builder);
	auto rhs = _inputs[1]->fetch_value(context, builder);
	auto p = builder.CreateFAdd(lhs, rhs);
	builder.CreateStore(p, _outputs[0]->fetch_value_raw(context, builder));
}

void SubReal::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
								 xerxzema::Program *program)
{
	auto lhs = _inputs[0]->fetch_value(context, builder);
	auto rhs = _inputs[1]->fetch_value(context, builder);
	auto p = builder.CreateFSub(lhs, rhs);
	builder.CreateStore(p, _outputs[0]->fetch_value_raw(context, builder));
}

void MulReal::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
								 xerxzema::Program *program)
{
	auto lhs = _inputs[0]->fetch_value(context, builder);
	auto rhs = _inputs[1]->fetch_value(context, builder);
	auto p = builder.CreateFMul(lhs, rhs);
	builder.CreateStore(p, _outputs[0]->fetch_value_raw(context, builder));
}

void DivReal::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
								 xerxzema::Program *program)
{
	auto lhs = _inputs[0]->fetch_value(context, builder);
	auto rhs = _inputs[1]->fetch_value(context, builder);
	auto p = builder.CreateFDiv(lhs, rhs);
	builder.CreateStore(p, _outputs[0]->fetch_value_raw(context, builder));
}

llvm::Type* Delay::state_type(llvm::LLVMContext &context)
{
	std::vector<llvm::Type*> data_types;
	data_types.push_back(llvm::Type::getInt1Ty(context));
	data_types.push_back(_inputs[0]->type()->type(context));

	_state_type = llvm::StructType::create(context, data_types);
	return _state_type;
}

};
