#include "Program.h"
#include "Namespace.h"
#include "World.h"
#include "LLVMUtils.h"
#include "llvm/IR/IRBuilder.h"
#include "Diagnostics.h"
#include "Parser.h"

namespace xerxzema
{
Program::Program(Namespace* p, const std::string& name) : parent(p), root_name(name),
														  is_trivial(false), valid(true),
														  call_site(nullptr)
{
	reg("head");
	reg("head")->type(p->world()->get_namespace("core")->type("unit"));
}

void Program::add_input(const std::string &name, xerxzema::Type *type)
{
	auto r = std::make_unique<Register>(name);
	r->type(type);
	inputs.push_back(r.get());
	registers.emplace(name, std::move(r));

}

std::string Program::symbol_name()
{
	return parent->full_name() + "." + root_name;
}

void Program::add_output(const std::string &name, xerxzema::Type *type)
{
	auto r = std::make_unique<Register>(name);
	r->type(type);
	outputs.push_back(r.get());
	registers.emplace(name, std::move(r));
}

Register* Program::reg(const std::string &name)
{
	if(registers.find(name) != registers.end())
		return registers[name].get();
	auto r = std::make_unique<Register>(name);
	auto result = r.get();
	registers.emplace(name, std::move(r));
	locals.push_back(result);
	return result;
}

void Program::instruction(std::unique_ptr<Instruction>&& inst)
{
	instructions.push_back(std::move(inst));
}

void Program::instruction(const std::string &name,
						  const std::vector<std::string> &inputs,
						  const std::vector<std::string> &outputs)
{
	std::vector<RegisterData> in_regs;
	std::vector<RegisterData> out_regs;

	for(auto& n:inputs)
	{
		in_regs.push_back(reg_data(n));
	}

	for(auto& n:outputs)
	{
		out_regs.push_back(reg_data(n));
	}
	instruction(name, in_regs, out_regs, {});
}

void Program::instruction(const std::string &name,
						  const std::vector<RegisterData> &inputs,
						  const std::vector<RegisterData> &outputs,
						  const std::vector<RegisterData> &dependencies,
						  Expression* source)
{

	if(!check_instruction(name, inputs, outputs, dependencies, source))
	{
		auto def = std::make_unique<DeferredInstruction>();
		def->name = name;
		def->inputs = inputs;
		def->outputs = outputs;
		def->dependencies = dependencies;
		def->solved = false;
		def->source = source;

		for(auto& r: inputs)
		{
			r.reg->deffered.push_back(def.get());
		}

		deferred.push_back(std::move(def));
	}
}

RegisterData Program::constant(double literal)
{
	auto inst = std::make_unique<ValueReal>(literal);
	inst->dependent(reg("head"));
	auto temp = temp_reg();
	temp.reg->type(parent->world()->get_namespace("core")->type("real"));
	inst->output(temp.reg);
	instruction(std::move(inst));
	temp.sample = true;
	return temp;
}

RegisterData Program::constant_int(int64_t literal)
{
	auto inst = std::make_unique<ValueInt>(literal);
	inst->dependent(reg("head"));
	auto temp = temp_reg();
	temp.reg->type(parent->world()->get_namespace("core")->type("int"));
	inst->output(temp.reg);
	instruction(std::move(inst));
	temp.sample = true;
	return temp;
}
bool Program::check_instruction(const std::string &name,
								const std::vector<RegisterData> &inputs,
								const std::vector<RegisterData> &outputs,
								const std::vector<RegisterData> &dependencies,
								Expression* source)
{
	bool resolved = true;
	std::vector<Type*> input_types;
	for(auto& reg_data: inputs)
	{
		if(!reg_data.reg->type())
		{
			resolved = false;
			break;
		}
		else
		{
			input_types.push_back(reg_data.reg->type());
		}
	}
	if(resolved)
	{
		auto def = parent->resolve_instruction(name, input_types);
		if(def)
		{
			auto output_types = def->output_types(parent);
			//more comments?
			//we can create extra outputs here if needed?
			//probably issue a warning or something
			std::vector<RegisterData> target_outputs(outputs);
			if(output_types.size() > target_outputs.size())
			{
				//create extra temporary registers to pad the outputs
				auto it = output_types.begin();
				it+= target_outputs.size();
				while(it != output_types.end())
				{
					target_outputs.push_back(temp_reg());
					it++;
				}
			}
			if(output_types.size() == target_outputs.size())
			{
				auto it = target_outputs.begin();
				for(auto& type: output_types)
				{
					auto out_reg = it->reg;
					if(out_reg->type())
					{
						if(out_reg->type() != type)
						{
							//invalid type, register is already type deduced
						}
					}
					else
					{
						out_reg->type(type);
						for(auto& retry:out_reg->deffered)
						{
							if(!retry->solved)
							{
								if(check_instruction(retry->name, retry->inputs, retry->outputs,
													 retry->dependencies, retry->source))
								{
									retry->solved = true;
								}
							}
						}
					}
					it++;
				}
				auto inst = def->create();
				for(auto& n:inputs)
				{
					if(n.sample)
						inst->sample(n.reg);
					else
						inst->input(n.reg);
				}
				for(auto& n:target_outputs)
				{
					inst->output(n.reg);
				}
				for(auto& n:dependencies)
				{
					inst->dependent(n.reg);
				}
				inst->validate_mask();
				instruction(std::move(inst));
				return true;
			}
			else
			{
				//output arg number mismatch.
			}
		}
		else
		{
			emit_error(source->token.get(), "Unknown call to: " + name);
		}
	}
	return false;
}

llvm::FunctionType* Program::function_type(llvm::LLVMContext& context)
{
	//WHY dont' we just spin-wait for the initial version of this
	//and try to get clever later that will really simplify
	//
	std::vector<llvm::Type*> data_types;
	data_types.push_back(llvm::Type::getInt1Ty(context));  //the "has initialized yet" flag
	data_types.push_back(llvm::Type::getInt32Ty(context)); //version data
	data_types.push_back(llvm::Type::getInt32Ty(context)); //ref counter
	data_types.push_back(llvm::Type::getInt32Ty(context)); //user counter
	data_types.push_back(llvm::Type::getInt64Ty(context)); //time when scheduled
	//we may want to promote the time-when-scheduled to a function argument
	int i = 5;
	for(auto r: locals)
	{
		if(!r->type())
		{
			emit_error("local symbol (" + r->name() + ") is undefined.");
			return nullptr;
		}

		if(r->type()->name() != "unit")
		{
			data_types.push_back(r->type()->type(context));
			r->offset(i++);
		}
	}
	for(auto& r: instructions)
	{
		data_types.push_back(llvm::Type::getInt16Ty(context));
		r->offset(i++);
		if(r->state_type(context))
		{
			data_types.push_back(r->state_type(context));
			i++;
		}
	}

	for(auto& r:inputs)
	{
		data_types.push_back(r->type()->type(context));
		r->offset(i++);
	}
	for(auto& r:outputs)
	{
		data_types.push_back(r->type()->type(context));
		r->offset(i++);
	}
	state_type = llvm::StructType::create(context, data_types, symbol_name() + ".state.data");

	std::vector<llvm::Type*> arg_types;
	arg_types.push_back(state_type->getPointerTo());

	return llvm::FunctionType::get(state_type->getPointerTo(), arg_types, false);
}


void Program::allocate_registers(llvm::LLVMContext& context, llvm::IRBuilder<>& builder,
								 llvm::Function* fn)
{
	auto args = fn->arg_begin();
	args++;
	for(auto r: inputs)
	{
		auto value = builder.CreateAlloca(r->type()->type(context), nullptr, r->name());
		r->value(value);
	}

	for(auto r: outputs)
	{
		auto value = builder.CreateAlloca(r->type()->type(context), nullptr, r->name());
		r->value(value);
	}

	for(auto r: locals)
	{
		if(r->type()->name() != "unit")
		{
			auto value = builder.CreateAlloca(r->type()->type(context), nullptr, r->name());
			r->value(value);
			r->type()->init(context, builder, value); //this should only be done during head
		}
	}
	for(auto& i: instructions)
	{
		i->value(builder.CreateAlloca(llvm::Type::getInt16Ty(context)));
		builder.CreateStore(const_int16(context, 0), i->value());
		auto instruction_state_type = i->state_type(context);
		if(instruction_state_type != nullptr)
		{
			i->state_value(builder.CreateAlloca(instruction_state_type));
		}
	}
	activation_counter = builder.CreateAlloca(llvm::Type::getInt64Ty(context), nullptr, "counter");
}

void Program::generate_exit_block(llvm::LLVMContext& context, llvm::IRBuilder<>& builder)
{
	for(auto& reg: registers)
	{
		auto r = reg.second.get();
		if(r->type()->name() != "unit")
		{
			auto ptr = builder.CreateStructGEP(state_type, &*function->arg_begin(), r->offset());
			r->type()->copy(context, builder, ptr, r->fetch_value_raw(context, builder));
		}
	}
	for(auto& r: instructions)
	{
		auto val = builder.CreateLoad(r->value());
		auto ptr = builder.CreateStructGEP(state_type, &*function->arg_begin(), r->offset());
		builder.CreateStore(val, ptr);

		auto instruction_state_type = r->state_type(context);
		if(instruction_state_type != nullptr)
		{
			auto sz = llvm::ConstantExpr::getSizeOf(instruction_state_type);
			auto dst_ptr = builder.CreateStructGEP(state_type, &*function->arg_begin(),
												   r->offset()+1);
			auto src_ptr = r->state_value();
			builder.CreateMemCpy(dst_ptr, src_ptr, sz, 0);
		}
	}
}

llvm::BasicBlock* Program::generate_entry_block(llvm::LLVMContext& context,
												llvm::IRBuilder<>& builder)
{
	auto state = &*function->arg_begin();
	auto entry_block = llvm::BasicBlock::Create(context, "entry", function);
	auto head_block = llvm::BasicBlock::Create(context, "head", function);
	auto resume_block = llvm::BasicBlock::Create(context, "resume", function);
	auto first_block = llvm::BasicBlock::Create(context, "first", function);
	auto input_activation_block = llvm::BasicBlock::Create(context, "input_activation", function);

	builder.SetInsertPoint(entry_block);
	allocate_registers(context, builder, function);
	auto ptr = builder.CreateStructGEP(state_type, &*function->arg_begin(), 0);
	auto reentry_val = builder.CreateLoad(ptr);
	builder.CreateCondBr(reentry_val, resume_block, head_block);

	builder.SetInsertPoint(head_block);
	reg("head")->do_activations(context, builder);
	for(auto r: inputs)
	{
		r->do_activations(context, builder);
	}
	ptr = builder.CreateStructGEP(state_type, &*function->arg_begin(), 0);
	builder.CreateStore(const_int1(context, 1), ptr);

	//create backing for i/o arguments when executing head.
	for(auto r: inputs)
	{
		if(r->type()->name() != "unit")
		{
			ptr = builder.CreateStructGEP(state_type, &*function->arg_begin(), r->offset());
			r->type()->copy(context, builder, r->fetch_value_raw(context, builder), ptr);
		}
	}
	for(auto r: outputs)
	{
		if(r->type()->name() != "unit")
		{
			ptr = builder.CreateStructGEP(state_type, &*function->arg_begin(), r->offset());
			r->type()->init(context, builder, ptr);
		}
	}

	//initialize any default instruction state
	for(auto& i: instructions)
	{
		auto instruction_state_type = i->state_type(context);
		if(instruction_state_type != nullptr)
		{
			i->generate_state_initializer(context, builder, this);
		}
	}

	builder.CreateBr(first_block);

	builder.SetInsertPoint(resume_block);

	for(auto& reg: registers)
	{
		auto r = reg.second.get();
		if(r->type()->name() != "unit")
		{
			ptr = builder.CreateStructGEP(state_type, &*function->arg_begin(), r->offset());
			r->type()->copy(context, builder,r->fetch_value_raw(context, builder), ptr);
		}
	}

	for(auto& r: instructions)
	{
		auto ptr = builder.CreateStructGEP(state_type, &*function->arg_begin(), r->offset());
		auto val = builder.CreateLoad(ptr);
		builder.CreateStore(val, r->value());

		auto instruction_state_type = r->state_type(context);
		if(instruction_state_type != nullptr)
		{
			auto sz = llvm::ConstantExpr::getSizeOf(instruction_state_type);
			auto src_ptr = builder.CreateStructGEP(state_type, &*function->arg_begin(),
												   r->offset()+1);
			auto dst_ptr = r->state_value();
			builder.CreateMemCpy(dst_ptr, src_ptr, sz, 0);
		}
	}
	builder.CreateBr(input_activation_block);

	builder.SetInsertPoint(input_activation_block);
	for(auto r: inputs)
	{
		r->do_activations(context, builder);
	}
	builder.CreateBr(first_block);
	return first_block;
}

llvm::Function* Program::trampoline_gen(llvm::Module* module, llvm::LLVMContext& context,
										llvm::GlobalVariable* target_call, const std::string& call_name)
{
	auto ftype = function->getFunctionType();
	auto trampoline = llvm::Function::Create
		(ftype, llvm::GlobalValue::LinkageTypes::ExternalLinkage, call_name, module);

	llvm::IRBuilder<> builder(context);
	auto entry_block = llvm::BasicBlock::Create(context, "entry", trampoline);
	auto lock_block =  llvm::BasicBlock::Create(context, "lock", trampoline);
	auto check_block = llvm::BasicBlock::Create(context, "check", trampoline);
	auto jump_block = llvm::BasicBlock::Create(context, "trampoline", trampoline);
	auto fix_block = llvm::BasicBlock::Create(context, "fix", trampoline);


	builder.SetInsertPoint(entry_block);
	builder.CreateBr(lock_block);
	builder.SetInsertPoint(lock_block);
	auto user_counter_ptr = builder.CreateStructGEP(state_type, &*trampoline->arg_begin(), 3);
	auto attempt_val = builder.CreateAtomicCmpXchg(user_counter_ptr, const_int32(context, 0),
												   const_int32(context, 1),
												   llvm::AtomicOrdering::AcquireRelease,
												   llvm::AtomicOrdering::Monotonic);
	auto succeded = builder.CreateExtractValue(attempt_val, 1);
	builder.CreateCondBr(succeded, check_block, lock_block);

	builder.SetInsertPoint(check_block);
	auto version_value = builder.CreateLoad(version_number);

	//stuff like this is very fragile...
	auto current_ptr = builder.CreateStructGEP(state_type, &*trampoline->arg_begin(), 1);
	auto current_value = builder.CreateLoad(current_ptr);
	auto compare = builder.CreateICmpEQ(current_value, version_value);
	builder.CreateCondBr(compare, jump_block, fix_block);


	builder.SetInsertPoint(fix_block);
	//spin wait here until there are no pending uses
	std::vector<llvm::Value*> fix_args;
	fix_args.push_back(&*trampoline->arg_begin());
	auto transform_value = builder.CreateLoad(transform_site);
	builder.CreateCall(transform_value, fix_args);
	builder.CreateBr(jump_block);

	builder.SetInsertPoint(jump_block);
	std::vector<llvm::Value*> args;
	args.push_back(&*trampoline->arg_begin());
	auto call_value = builder.CreateLoad(target_call);
	auto ret = builder.CreateCall(call_value, args);


	builder.CreateAtomicRMW(llvm::AtomicRMWInst::Sub, user_counter_ptr,
							const_int32(context, 1), llvm::AtomicOrdering::AcquireRelease);

	builder.CreateRet(ret);

	return trampoline;
}

void Program::transform_gen(llvm::Module* module, llvm::LLVMContext& context)
{
	//just generate the default transformer for right now
	auto ftype = function->getFunctionType();
	transformer = llvm::Function::Create
		(ftype, llvm::GlobalValue::LinkageTypes::ExternalLinkage, symbol_name() + ".transformer.impl0", module);

	llvm::IRBuilder<> builder(context);
	auto bb = llvm::BasicBlock::Create(context, "transform", transformer);
	builder.SetInsertPoint(bb);
	//this should actually return a malloc'd state structure
	builder.CreateRet(&*transformer->arg_begin());
}

llvm::Function* Program::create_declaration(llvm::Module* module, llvm::LLVMContext& context)
{

	auto ftype = function_type(context);
	if(!ftype)
	{
		//report this somehow...
		return nullptr;
	}
	return llvm::Function::Create(ftype,
								  llvm::GlobalValue::LinkageTypes::ExternalLinkage,
								  symbol_name(), module);
}

llvm::Function* Program::create_dtor_declaration(llvm::Module* module, llvm::LLVMContext& context)
{
	auto ftype = function_type(context);
	if(!ftype)
	{
		//report this somehow...
		return nullptr;
	}
	return llvm::Function::Create(ftype,
								  llvm::GlobalValue::LinkageTypes::ExternalLinkage,
								  symbol_name() + ".dtor", module);
}

void Program::code_gen(llvm::Module *module, llvm::LLVMContext &context)
{
	auto ftype = function_type(context);
	if(!ftype)
	{
		emit_error("aborting codegen for " + symbol_name());
		valid = false;
		return;
	}
	emit_debug("codegen for: " + symbol_name());
	function = llvm::Function::Create(ftype,
									 llvm::GlobalValue::LinkageTypes::ExternalLinkage,
									 symbol_name() + ".impl.0" , module);

	if(!call_site)
	{
		call_site = new llvm::GlobalVariable(*module, ftype->getPointerTo(), false,
											 llvm::GlobalVariable::LinkageTypes::ExternalLinkage,
											 function, symbol_name() + ".call_site");
		transform_gen(module, context);
		transform_site = new llvm::GlobalVariable(*module, ftype->getPointerTo(), false,
											 llvm::GlobalVariable::LinkageTypes::ExternalLinkage,
											 transformer, symbol_name() + ".transform_site");

		version_number = new llvm::GlobalVariable
			(*module, llvm::Type::getInt32Ty(context), false,
			 llvm::GlobalVariable::LinkageTypes::ExternalLinkage,
			 const_int32(context, 0), symbol_name() + ".version_number");

		trampoline_entry = trampoline_gen(module, context, call_site, symbol_name());
	}

	_current_module = module;

	llvm::IRBuilder<> builder(context);

	program_state = &*function->arg_begin();

	auto post_entry_block = generate_entry_block(context, builder);
	auto tail_block = llvm::BasicBlock::Create(context, "tail", function);

	builder.SetInsertPoint(post_entry_block);


	llvm::BasicBlock* next_condition = nullptr;
	llvm::BasicBlock* condition = nullptr;
	llvm::BasicBlock* op_block = nullptr;
	llvm::BasicBlock* first_block = nullptr;

	if(!instructions.size())
	{
		builder.CreateBr(tail_block);
		first_block = tail_block;
	}

	int inst = 0;
	auto it = instructions.begin();
	while(it != instructions.end())
	{
		if(next_condition != nullptr)
		{
			condition = next_condition;
		}
		else
		{
			auto cond_block_name = (*it)->name() + "_cond";
			condition = llvm::BasicBlock::Create(context, cond_block_name, function);
			first_block = condition;
			builder.CreateBr(condition);
		}
		if(inst + 1 == instructions.size())
		{
			next_condition = tail_block;
		}
		else
		{
			next_condition = llvm::BasicBlock::Create(context, (*(it+1))->name() + "_cond", function);
		}
		auto op_block_name = (*it)->name() + "_op";
		op_block = llvm::BasicBlock::Create(context, op_block_name, function);
		(*it)->generate_check(context, builder, this, condition,op_block, next_condition);
		builder.SetInsertPoint(op_block);
		(*it)->generate_operation(context, builder, this);
		(*it)->generate_prolouge(context, builder, this, next_condition);
		inst++;
		it++;
	}

	builder.SetInsertPoint(tail_block);
	auto reenter_block = llvm::BasicBlock::Create(context, "reenter", function);
	auto exit_block = llvm::BasicBlock::Create(context, "exit", function);
	auto counter_value = builder.CreateLoad(activation_counter);
	auto reenter = builder.CreateICmp(llvm::CmpInst::Predicate::ICMP_EQ, counter_value,
										llvm::ConstantInt::get(context, llvm::APInt(64, 0)));
	auto tail_ret = builder.CreateCondBr(reenter, exit_block, reenter_block);

	builder.SetInsertPoint(reenter_block);
	builder.CreateStore(llvm::ConstantInt::get(context, llvm::APInt(64, 0)), activation_counter);
	builder.CreateBr(first_block);

	builder.SetInsertPoint(exit_block);
	generate_exit_block(context, builder);
	builder.CreateRet(program_state);
	destructor_gen(module, context);
}

llvm::Value* Program::create_closure(xerxzema::Register *reg, bool reinvoke,
									 llvm::LLVMContext& context, llvm::Module* module)
{
	std::vector<llvm::Type*> arg_types;
	arg_types.push_back(state_type->getPointerTo());

	if(reg->type()->name() != "unit")
		arg_types.push_back(reg->type()->type(context)->getPointerTo());

	auto closure_type = llvm::FunctionType::get(state_type->getPointerTo(),
												arg_types, false);

	auto fn = llvm::Function::Create(closure_type,
									 llvm::GlobalValue::LinkageTypes::ExternalLinkage,
									 symbol_name() + ".closure." + reg->name() + ".impl0", module);

	auto closure_var  = new llvm::GlobalVariable(*module, closure_type->getPointerTo(), false,
												 llvm::GlobalVariable::LinkageTypes::ExternalLinkage,
												 fn,
												 symbol_name() + ".closure." + reg->name() + ".call_site");

	//TODO get this to work with extra input variables.
	auto wrapper = trampoline_gen(module, context, closure_var, symbol_name() + ".closure." + reg->name());

	llvm::Value* arg_ptr = nullptr;
	auto args = fn->arg_begin();

	auto state = &*args++;
	if(reg->type()->name() != "unit")
	{
		arg_ptr = &*args++;
	}


	llvm::IRBuilder<> builder(context);
	auto block = llvm::BasicBlock::Create(context, "entry", fn);
	builder.SetInsertPoint(block);

	if(reg->type()->name() != "unit")
	{
		auto dest_ptr = builder.CreateStructGEP(state_type, state, reg->offset());
		reg->type()->copy(context, builder, dest_ptr, arg_ptr);
	}

	//update activation masks
	for(auto& activate: reg->activations)
	{
		auto ptr = builder.CreateStructGEP(state_type, state,
										   activate.instruction->offset());
		auto mask = llvm::ConstantInt::get(llvm::Type::getInt16Ty(context), activate.value);
		auto val = builder.CreateLoad(ptr);
		auto new_val = builder.CreateOr(val, mask);
		builder.CreateStore(new_val, ptr);
	}

	if(reinvoke)
	{
		//decrement the lock counter
		auto user_counter_ptr = builder.CreateStructGEP(state_type, &*fn->arg_begin(), 3);
		builder.CreateAtomicRMW(llvm::AtomicRMWInst::Sub, user_counter_ptr,
							const_int32(context, 1), llvm::AtomicOrdering::AcquireRelease);

		//call the original function back with the bound io values...
		std::vector<llvm::Value*> args;
		args.push_back(state);
		auto ret_val = builder.CreateCall(trampoline_entry, args);
		builder.CreateAtomicRMW(llvm::AtomicRMWInst::Add, user_counter_ptr,
							const_int32(context, 1), llvm::AtomicOrdering::AcquireRelease);
		builder.CreateRet(ret_val);
	}
	else
	{
		builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), 0));
	}

	return wrapper;
}


void Program::destructor_gen(llvm::Module* module, llvm::LLVMContext& context)
{
	std::vector<llvm::Type*> arg_types;
	arg_types.push_back(state_type->getPointerTo());

	llvm::FunctionType::get(state_type->getPointerTo(),
							arg_types, false);

	auto closure_type = llvm::FunctionType::get(state_type->getPointerTo(),
												arg_types, false);

	auto fn = llvm::Function::Create(closure_type,
									 llvm::GlobalValue::LinkageTypes::ExternalLinkage,
									 symbol_name() + ".dtor.impl0", module);

	auto closure_var  = new llvm::GlobalVariable(*module, closure_type->getPointerTo(), false,
												 llvm::GlobalVariable::LinkageTypes::ExternalLinkage,
												 fn,
												 symbol_name() + ".dtor.call_site");

	auto wrapper = trampoline_gen(module, context, closure_var, symbol_name() + ".dtor");

	auto state = &*fn->arg_begin();

	llvm::IRBuilder<> builder(context);
	auto block = llvm::BasicBlock::Create(context, "entry", fn);
	builder.SetInsertPoint(block);

	for(auto& r:registers)
	{
		auto reg = r.second.get();
		if(reg->type()->name() != "unit")
		{
			auto site_ptr = builder.CreateStructGEP(state_type, state, reg->offset());
			reg->type()->destroy(context, builder, site_ptr);
		}
	}
	for(auto& inst:instructions)
	{
		if(inst->state_type(context) != nullptr)
		{
			auto site_ptr = builder.CreateStructGEP(state_type, state, inst->offset()+1);
			inst->generate_state_destructor(context, builder, this, site_ptr);
		}
	}
	builder.CreateRet(state);
}

};
