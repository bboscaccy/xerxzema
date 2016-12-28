#include "Instruction.h"
#include "Register.h"
#include "Program.h"
#include "LLVMUtils.h"
#include <sstream>
#include "Namespace.h"
#include "Diagnostics.h"

#include "llvm/IR/Constants.h"

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

std::string Instruction::description()
{
	std::stringstream ss;
	ss << name() << "(";
	ss << constant_description() << " ";
	for(auto& reg:_inputs)
	{
		ss << reg->name();
		if(reg->type())
			ss << ':' << reg->type()->name() << ' ';
		else
			ss << ":null ";
	}
	ss << ") -> ( ";
	for(auto& reg:_outputs)
	{
		ss << reg->name();
		if(reg->type())
			ss << ':' << reg->type()->name() << ' ';
		else
			ss << ":null ";
	}
	ss << ") { deps: ";
	for(auto& reg:_deps)
	{
		ss << reg->name();
		if(reg->type())
			ss << ':' << reg->type()->name() << ' ';
		else
			ss << ":null ";
	}
	ss << "}";
	ss << " activate: " << mask;
	ss << " reset: " << reset_mask;
	return ss.str();
}

std::string Instruction::diff_description()
{
	bool include_outputs = false;
	std::stringstream ss;
	ss << name() << "(";
	auto const_desc = constant_description();
	if(const_desc.size() > 0)
	{
		ss << " " << constant_description() << " ";
		include_outputs = true;
	}
	for(auto& reg:_inputs)
	{
		ss << reg->name();
		if(reg->type())
			ss << ':' << reg->type()->name() << ' ';
		else
			ss << ":null ";
	}
	ss << ") -> ( ";

	if(include_outputs)
	{
		for(auto& reg:_outputs)
		{
			ss << reg->name();
			if(reg->type())
				ss << ':' << reg->type()->name() << ' ';
			else
				ss << ":null ";
		}
	}
	else
	{
		ss << "... ";
	}
	ss << ") { deps: ";
	for(auto& reg:_deps)
	{
		ss << reg->name();
		if(reg->type())
			ss << ':' << reg->type()->name() << ' ';
		else
			ss << ":null ";
	}
	ss << "}";
	return ss.str();
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

void Instruction::generate_state_destructor(llvm::LLVMContext &context,
											llvm::IRBuilder<> &builder,
											xerxzema::Program *program,
											llvm::Value* state_ptr)
{
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


ProgramDirectCall::ProgramDirectCall(Program* target) : target(target) {}
llvm::Type* ProgramDirectCall::state_type(llvm::LLVMContext &context)
{

	if(_state_type)
		return _state_type;
	_state_type = target->state_type_value()->getPointerTo();
	return _state_type;
}

void ProgramDirectCall::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
										   xerxzema::Program *program)
{
	auto fn = program->current_module()->getFunction(target->symbol_name());
	if(!fn)
	{
		fn = program->create_declaration(program->current_module(), context);
	}

	//state_value locally is an alloca so that makes it a pointer to a pointer.
	auto state = builder.CreateLoad(state_value());
	auto in_counter = 0;
	for(auto& reg:_inputs)
	{
		auto program_offset = target->input_registers()[in_counter]->offset();
		auto value_ptr = builder.CreateStructGEP(target->state_type_value(), state, program_offset);
		reg->type()->copy(context, builder, value_ptr, reg->fetch_value_raw(context, builder));
		in_counter++;
	}
	auto call_ret = builder.CreateCall(fn, {state});
	//TODO once we hook up return states from functions
	//assign this...
	auto out_counter = 0;
	for(auto& reg:_outputs)
	{
		auto program_offset = target->output_registers()[out_counter]->offset();
		auto value_ptr = builder.CreateStructGEP(target->state_type_value(), state, program_offset);
		reg->type()->copy(context, builder, reg->fetch_value_raw(context, builder), value_ptr);
		out_counter++;
	}
	builder.CreateStore(call_ret, state_value());
}

std::string ProgramDirectCall::name()
{
	return "call." + target->symbol_name();
}

void ProgramDirectCall::generate_state_initializer(llvm::LLVMContext &context,
												   llvm::IRBuilder<> &builder,
												   xerxzema::Program *program)
{
	auto size = llvm::ConstantExpr::getSizeOf(target->state_type_value());
	auto fn = program->name_space()->get_external_function("malloc", program->current_module(), context);
	auto result = builder.CreateCall(fn, {size});
	builder.CreateMemSet(result, builder.getInt8(0), size, 0);
	auto ptr = builder.CreateBitCast(result, state_type(context));
	builder.CreateStore(ptr, state_value());
}

void ProgramDirectCall::generate_state_destructor(llvm::LLVMContext &context,
												  llvm::IRBuilder<> &builder,
												  xerxzema::Program *program,
												  llvm::Value* state_ptr)
{
	auto value = builder.CreateLoad(state_ptr);

	//call the dtor on this pointer.
	auto dtorfn = program->current_module()->getFunction(target->symbol_name() + ".dtor");
	if(!dtorfn)
	{
		dtorfn = program->create_dtor_declaration(program->current_module(), context);
	}

	builder.CreateCall(dtorfn, {value});

	auto fn = program->name_space()->get_external_function("free", program->current_module(), context);
	auto cast = builder.CreateBitCast(value, llvm::Type::getInt8PtrTy(context));
	builder.CreateCall(fn, {cast});
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
												{llvm::Type::getDoubleTy(context)});
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

	auto format_addr = builder.CreateInBoundsGEP(format_str,
										 {builder.getInt32(0), builder.getInt32(0)});

	auto fn = program->name_space()->get_external_function("print", program->current_module(), context);
	builder.CreateCall(fn, {format_addr, _inputs[0]->fetch_value(context, builder)});
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
	auto fn = program->name_space()->get_external_function
		("schedule", program->current_module(), context);
	auto closure = program->create_closure(_outputs[0], true,
										   context, program->current_module());
	auto scheduler_var = program->name_space()->get_external_variable
		("scheduler", program->current_module(), context);

	auto scheduler = builder.CreateLoad(scheduler_var);
	auto time = _inputs[0]->fetch_value(context, builder);
	auto state = program->current_state();
	auto state_cast = builder.CreateBitCast(state, llvm::Type::getInt8PtrTy(context));
	auto closure_cast = builder.CreateBitCast(closure, llvm::Type::getInt8PtrTy(context));
	builder.CreateCall(fn, {scheduler, closure_cast, state_cast, time});
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

ArrayBuilder::ArrayBuilder(Type* array_type) : array_type(array_type)
{
}

void ArrayBuilder::generate_operation(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
									  xerxzema::Program *program)
{
	auto out_struct = _outputs[0]->fetch_value_raw(context, builder); //get a pointer to this
	auto data_ptr = builder.CreateStructGEP(_outputs[0]->type()->type(context),
											out_struct, 0);
	auto val_ptr = builder.CreateLoad(data_ptr);
	auto is_null = builder.CreateICmpEQ(val_ptr,
										llvm::ConstantPointerNull::get
										(array_type->type(context)->getPointerTo()));

	auto kill_block = llvm::BasicBlock::Create(context, "kill_array", program->function_value());
	auto create_block = llvm::BasicBlock::Create(context, "create_array", program->function_value());

	builder.CreateCondBr(is_null, create_block, kill_block);

	builder.SetInsertPoint(kill_block);


	auto mallocator = program->name_space()->get_external_function("malloc",
																   program->current_module(),
																   context);
	//TODO move this to type destructor
	auto deallocator = program->name_space()->get_external_function("free",
																	program->current_module(),
																	context);
	if(array_type->is_trivial())
	{
		builder.CreateCall(deallocator, {val_ptr});
		auto sz = llvm::ConstantExpr::getSizeOf(_outputs[0]->type()->type(context));
		builder.CreateMemSet(out_struct, builder.getInt8(0), sz, 0);
		builder.CreateBr(create_block);
	}
	else
	{
		//for each blah, destroy blah

	}

	//TODO
	// we need an "after-head" sort of optimnization pass that can detect which items are only activated
	// by head-fired registers and remove extra mask updates
	auto alloc_size = llvm::ConstantExpr::getMul
		(llvm::ConstantExpr::getSizeOf(array_type->type(context)),
									   builder.getInt64(_inputs.size()));
	auto new_ptr = builder.CreateCall(mallocator, {alloc_size });
	auto cast = builder.CreatePointerCast(new_ptr, array_type->type(context)->getPointerTo());
	builder.CreateStore(cast, data_ptr);
	auto dst_ptr = builder.CreateLoad(data_ptr);
	auto next_ptr = builder.CreateGEP(dst_ptr, builder.getInt32(0));
	for(size_t i = 0; i < _inputs.size(); i++)
	{
		//TODO detect single-use copy or move?
		array_type->copy(context, builder, dst_ptr, _inputs[0]->fetch_value(context,
																			builder));
		next_ptr = builder.CreateGEP(dst_ptr, builder.getInt32(i));
	}

}

};
