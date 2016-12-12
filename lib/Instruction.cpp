#include "Instruction.h"
#include "Register.h"
#include "Program.h"
#include "LLVMUtils.h"

namespace xerxzema
{

Instruction::Instruction() :  _offset(0), mask(0), reset_mask(0), _state_type(nullptr),
							  _value(nullptr), _state_value(nullptr), _eof_value(nullptr)
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
	mask |= (1 << _deps.size());
	reset_mask |= (1 << _deps.size());
	_deps.push_back(reg);
}

void Instruction::oneshot_dependent(xerxzema::Register *reg)
{
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

void Merge::generate_check(llvm::LLVMContext& context,
								 llvm::IRBuilder<> &builder,
								 Program* program,
								 llvm::BasicBlock* check_block,
								 llvm::BasicBlock* op_block,
								 llvm::BasicBlock* next_block)
{
	builder.SetInsertPoint(check_block);
	auto mask_value = builder.CreateLoad(_value);
	auto comp_value = builder.CreateICmp(llvm::CmpInst::Predicate::ICMP_UGT,
										 mask_value,
										 llvm::ConstantInt::get(context, llvm::APInt(16, 0)));
	builder.CreateCondBr(comp_value, op_block, next_block);
}

void Merge::generate_operation(llvm::LLVMContext &context,
							   llvm::IRBuilder<> &builder,
							   xerxzema::Program *program)
{

}

void Merge::generate_prolouge(llvm::LLVMContext &context,
							  llvm::IRBuilder<> &builder,
							  Program* program,
							  llvm::BasicBlock *next_block)
{
	builder.CreateStore(llvm::ConstantInt::get(context, llvm::APInt(16, reset_mask)), _value);
	auto p = builder.CreateLoad(program->activation_counter_value());
	auto i = builder.CreateAdd(p, llvm::ConstantInt::get(context, llvm::APInt(64,1)));
	builder.CreateStore(i, program->activation_counter_value());

	auto arg1_block = llvm::BasicBlock::Create(context, "merge_arg1", program->function_value());
	auto arg0_block = llvm::BasicBlock::Create(context, "merge_arg0", program->function_value());

	auto mask_value = builder.CreateLoad(_value);
	auto arg0 = builder.CreateAnd(mask_value, 1);
	auto comp_value = builder.CreateICmpUGT(arg0,
											 llvm::ConstantInt::get(context, llvm::APInt(16, 0)));

	builder.CreateCondBr(comp_value, arg0_block, arg1_block);

	builder.SetInsertPoint(arg0_block);
	_inputs[0]->type()->copy(context, builder, _outputs[0]->fetch_value_raw(context, builder),
							 _inputs[0]->fetch_value_raw(context, builder));

	for(auto& r:_outputs)
	{
		r->do_activations(context, builder);
	}
	builder.CreateBr(next_block);

	builder.SetInsertPoint(arg1_block);
	_inputs[1]->type()->copy(context, builder, _outputs[0]->fetch_value_raw(context, builder),
							 _inputs[1]->fetch_value_raw(context, builder));

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
	auto p = builder.CreateStore(const_value, target);
}

ValueInt::ValueInt(int64_t v):value(v)	{}
void ValueInt::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
								   Program* program)
{
	auto const_value = const_int64(context, value);
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

void PowReal::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
								 xerxzema::Program *program)
{
	auto lhs = _inputs[0]->fetch_value(context, builder);
	auto rhs = _inputs[1]->fetch_value(context, builder);
	auto call = llvm::Intrinsic::getDeclaration(program->current_module(), llvm::Intrinsic::pow,
												{llvm::Type::getDoubleTy(context),
														llvm::Type::getDoubleTy(context)});
	auto p = builder.CreateCall(call, {rhs, lhs});
	builder.CreateStore(p, _outputs[0]->fetch_value_raw(context, builder));
}

void EqReal::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
								 xerxzema::Program *program)
{
	auto lhs = _inputs[0]->fetch_value(context, builder);
	auto rhs = _inputs[1]->fetch_value(context, builder);
	auto p = builder.CreateFCmpOEQ(lhs, rhs);
	builder.CreateStore(p, _outputs[0]->fetch_value_raw(context, builder));
}

void NeReal::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
								 xerxzema::Program *program)
{
	auto lhs = _inputs[0]->fetch_value(context, builder);
	auto rhs = _inputs[1]->fetch_value(context, builder);
	auto p = builder.CreateFCmpONE(lhs, rhs);
	builder.CreateStore(p, _outputs[0]->fetch_value_raw(context, builder));
}

void LtReal::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
								 xerxzema::Program *program)
{
	auto lhs = _inputs[0]->fetch_value(context, builder);
	auto rhs = _inputs[1]->fetch_value(context, builder);
	auto p = builder.CreateFCmpOLT(lhs, rhs);
	builder.CreateStore(p, _outputs[0]->fetch_value_raw(context, builder));
}

void LeReal::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
								 xerxzema::Program *program)
{
	auto lhs = _inputs[0]->fetch_value(context, builder);
	auto rhs = _inputs[1]->fetch_value(context, builder);
	auto p = builder.CreateFCmpOLE(lhs, rhs);
	builder.CreateStore(p, _outputs[0]->fetch_value_raw(context, builder));
}

void GtReal::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
								 xerxzema::Program *program)
{
	auto lhs = _inputs[0]->fetch_value(context, builder);
	auto rhs = _inputs[1]->fetch_value(context, builder);
	auto p = builder.CreateFCmpOGT(lhs, rhs);
	builder.CreateStore(p, _outputs[0]->fetch_value_raw(context, builder));
}

void GeReal::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
								 xerxzema::Program *program)
{
	auto lhs = _inputs[0]->fetch_value(context, builder);
	auto rhs = _inputs[1]->fetch_value(context, builder);
	auto p = builder.CreateFCmpOGE(lhs, rhs);
	builder.CreateStore(p, _outputs[0]->fetch_value_raw(context, builder));
}

llvm::Type* Delay::state_type(llvm::LLVMContext &context)
{

	if(_state_type)
		return _state_type;

	std::vector<llvm::Type*> data_types;
	data_types.push_back(llvm::Type::getInt1Ty(context));
	data_types.push_back(_inputs[0]->type()->type(context));

	_state_type = llvm::StructType::create(context, data_types);
	return _state_type;
}

void Delay::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
							   Program *program)
{

}

void Delay::generate_prolouge(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
							  Program *program, llvm::BasicBlock *next_block)
{

	builder.CreateStore(llvm::ConstantInt::get(context, llvm::APInt(16, reset_mask)), _value);


	auto fire_block = llvm::BasicBlock::Create(context, "fire_delay", program->function_value());
	auto skip_block = llvm::BasicBlock::Create(context, "skip_delay", program->function_value());

	auto bool_ptr = builder.CreateStructGEP(_state_type, _state_value, 0);
	auto bool_val = builder.CreateLoad(bool_ptr);
	builder.CreateCondBr(bool_val, fire_block, skip_block);

	builder.SetInsertPoint(skip_block);
	bool_ptr = builder.CreateStructGEP(_state_type, _state_value, 0);
	builder.CreateStore(llvm::ConstantInt::get(bool_val->getType(), 1), bool_ptr);
	auto delay_val_ptr = builder.CreateStructGEP(_state_type, _state_value, 1);
	_inputs[0]->type()->copy(context, builder, delay_val_ptr,
							 _inputs[0]->fetch_value_raw(context, builder));
	builder.CreateBr(next_block);

	builder.SetInsertPoint(fire_block);
	delay_val_ptr = builder.CreateStructGEP(_state_type, _state_value, 1);
	auto in_ptr = _inputs[0]->fetch_value_raw(context, builder);
	auto out_ptr = _outputs[0]->fetch_value_raw(context, builder);
	_inputs[0]->type()->copy(context, builder, out_ptr, delay_val_ptr);
	_inputs[0]->type()->copy(context, builder, delay_val_ptr, in_ptr);

	auto p = builder.CreateLoad(program->activation_counter_value());
	auto i = builder.CreateAdd(p, llvm::ConstantInt::get(context, llvm::APInt(64,1)));
	builder.CreateStore(i, program->activation_counter_value());

	for(auto& r:_outputs)
	{
		r->do_activations(context, builder);
	}
	builder.CreateBr(next_block);
}

void When::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
							  Program *program)
{

}

void When::generate_prolouge(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
							  Program *program, llvm::BasicBlock *next_block)
{
	auto true_block = llvm::BasicBlock::Create(context, "when_true", program->function_value());
	auto false_block = llvm::BasicBlock::Create(context, "when_false", program->function_value());

	builder.CreateStore(llvm::ConstantInt::get(context, llvm::APInt(16, reset_mask)), _value);
	auto p = builder.CreateLoad(program->activation_counter_value());
	auto i = builder.CreateAdd(p, llvm::ConstantInt::get(context, llvm::APInt(64,1)));
	builder.CreateStore(i, program->activation_counter_value());
	auto test_val = _inputs[0]->fetch_value(context, builder);
	builder.CreateCondBr(test_val, true_block, false_block);

	builder.SetInsertPoint(true_block);
	_inputs[1]->type()->copy(context, builder, _outputs[0]->fetch_value_raw(context, builder),
							 _inputs[1]->fetch_value_raw(context, builder));

	for(auto& r:_outputs)
	{
		r->do_activations(context, builder);
	}
	builder.CreateBr(next_block);

	builder.SetInsertPoint(false_block);

	for(auto& r:_outputs)
	{
		r->do_activations(context, builder);
	}
	builder.CreateBr(next_block);

}

void Cond::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
							  Program *program)
{

}

void Cond::generate_prolouge(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
							  Program *program, llvm::BasicBlock *next_block)
{
	auto true_block = llvm::BasicBlock::Create(context, "cond_true", program->function_value());
	auto false_block = llvm::BasicBlock::Create(context, "cond_false", program->function_value());

	builder.CreateStore(llvm::ConstantInt::get(context, llvm::APInt(16, reset_mask)), _value);
	auto test_val = _inputs[0]->fetch_value(context, builder);
	builder.CreateCondBr(test_val, true_block, false_block);

	builder.SetInsertPoint(true_block);
	auto p = builder.CreateLoad(program->activation_counter_value());
	auto i = builder.CreateAdd(p, llvm::ConstantInt::get(context, llvm::APInt(64,1)));
	builder.CreateStore(i, program->activation_counter_value());
	if(_inputs.size() > 1)
	{
		_inputs[1]->type()->copy(context, builder,
								 _outputs[0]->fetch_value_raw(context, builder),
								 _inputs[1]->fetch_value_raw(context, builder));
	}
	for(auto& r:_outputs)
	{
		r->do_activations(context, builder);
	}
	builder.CreateBr(next_block);

	builder.SetInsertPoint(false_block);
	builder.CreateBr(next_block);

}

void Bang::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
							  Program *program)
{

}

void Trace::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
							  Program *program)
{
	std::string input_str("(");
	input_str += _inputs[0]->name();
	input_str += ") %f\n";
	auto format_str = builder.CreateGlobalString(input_str);


	auto fn = program->current_module()->getFunction("xerxzema_print");
	if(!fn)
	{
		std::vector<llvm::Type*> printf_arg_types;
		printf_arg_types.push_back(llvm::Type::getInt8PtrTy(context));

		llvm::FunctionType* printf_type =
			llvm::FunctionType::get(llvm::Type::getInt32Ty(context), printf_arg_types, true);

		fn = llvm::Function::Create(printf_type, llvm::Function::ExternalLinkage,
									"xerxzema_print", program->current_module());

		fn->setCallingConv(llvm::CallingConv::C);
	}
	builder.CreateCall(fn, {format_str, _inputs[0]->fetch_value(context, builder)});
}

void Trace::generate_prolouge(llvm::LLVMContext &context,
									llvm::IRBuilder<> &builder,
									Program* program,
									llvm::BasicBlock *next_block)
{
	builder.CreateStore(llvm::ConstantInt::get(context, llvm::APInt(16, reset_mask)), _value);
	builder.CreateBr(next_block);
}

void Schedule::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
							  Program *program)
{
	auto fn = program->current_module()->getFunction("xerxzema_schedule");
	auto closure = program->create_closure(_outputs[0], true,
										   context, program->current_module());
	auto scheduler_var = program->current_module()->getGlobalVariable("xerxzema_scheduler");
	auto scheduler = builder.CreateLoad(scheduler_var);
	auto time = _inputs[0]->fetch_value(context, builder);
	auto state = program->current_state();
	builder.CreateCall(fn, {scheduler, closure, state, time});
}

void Schedule::generate_prolouge(llvm::LLVMContext &context,
									llvm::IRBuilder<> &builder,
									Program* program,
									llvm::BasicBlock *next_block)
{
	builder.CreateStore(llvm::ConstantInt::get(context, llvm::APInt(16, reset_mask)), _value);
	builder.CreateBr(next_block);
}

void Instruction::validate_mask()
{
	if(reset_mask == mask)
		reset_mask = 0;
}

};
