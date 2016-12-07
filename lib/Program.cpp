#include "Program.h"
#include "Namespace.h"
#include "World.h"
#include "llvm/IR/IRBuilder.h"

namespace xerxzema
{
Program::Program(Namespace* p, const std::string& name) : parent(p), _name(name)
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
			//invalid/unknown instruction
		}
	}
	return false;
}

llvm::FunctionType* Program::function_type(llvm::LLVMContext& context)
{
	std::vector<llvm::Type*> data_types;
	data_types.push_back(llvm::Type::getInt1Ty(context)); //state value
	int i = 1;
	for(auto r: locals)
	{
		if(!r->type())
		{
			printf("%s is undefined\n", r->name().c_str());
		}

		if(r->type()->name() != "unit")
		{
			data_types.push_back(r->type()->type(context));
			r->offset(i);
			i++;
		}
	}
	for(auto& r: instructions)
	{
		data_types.push_back(llvm::Type::getInt16Ty(context));
		r->offset(i);
		i++;
		if(r->state_type(context))
		{
			data_types.push_back(r->state_type(context));
			i++;
		}
	}

	state_type = llvm::StructType::create(context, data_types, _name + "_data");

	std::vector<llvm::Type*> arg_types;
	arg_types.push_back(state_type->getPointerTo());
	for(auto r: inputs)
	{
		arg_types.push_back(r->type()->type(context)->getPointerTo());
	}
	for(auto r: outputs)
	{
		arg_types.push_back(r->type()->type(context)->getPointerTo());
	}

	return llvm::FunctionType::get(llvm::Type::getInt64Ty(context), arg_types, false);
}


void Program::allocate_registers(llvm::LLVMContext& context, llvm::IRBuilder<>& builder,
								 llvm::Function* fn)
{
	auto it = fn->arg_begin();
	it++;
	for(auto r: inputs)
	{
		r->value(&*it);
		it++;
	}

	for(auto r: outputs)
	{
		r->value(&*it);
		it++;
	}

	for(auto r: locals)
	{
		auto value = builder.CreateAlloca(r->type()->type(context), nullptr, r->name());
		r->value(value);
		r->type()->init(context, builder, value);
	}
	for(auto& i: instructions)
	{
		i->value(builder.CreateAlloca(llvm::Type::getInt16Ty(context)));
		builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt16Ty(context), 0), i->value());
		auto instruction_state_type = i->state_type(context);
		if(instruction_state_type != nullptr)
		{
			i->state_value(builder.CreateAlloca(instruction_state_type));
			i->generate_state_initializer(context, builder, this);
		}
	}
	activation_counter = builder.CreateAlloca(llvm::Type::getInt64Ty(context), nullptr, "counter");
}

void Program::generate_exit_block(llvm::LLVMContext& context, llvm::IRBuilder<>& builder)
{
	for(auto r: locals)
	{
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
	builder.CreateStore(llvm::ConstantInt::get(context, llvm::APInt(1, 1)), ptr);

	//TODO after HEAD runs we need to clone our main instruction_state for each
	//i/o thread state and place it "somewhere" not an alloca's block since
	//this function will exit usually
	//so next up, create i/o ports and hook up the scheduler..

	builder.CreateBr(first_block);

	builder.SetInsertPoint(resume_block);
	for(auto r: locals)
	{
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
	for(auto r: inputs)
	{
		r->do_activations(context, builder);
	}
	builder.CreateBr(first_block);
	return first_block;
}

void Program::code_gen(llvm::Module *module, llvm::LLVMContext &context)
{
	auto ftype = function_type(context);
	function = llvm::Function::Create(ftype,
									 llvm::GlobalValue::LinkageTypes::ExternalLinkage,
									 _name , module);
	_current_module = module;

	llvm::IRBuilder<> builder(context);
	auto state = &*function->arg_begin();

	auto post_entry_block = generate_entry_block(context, builder);
	auto tail_block = llvm::BasicBlock::Create(context, "tail", function);
	builder.SetInsertPoint(post_entry_block);


	llvm::BasicBlock* next_condition = nullptr;
	llvm::BasicBlock* condition = nullptr;
	llvm::BasicBlock* op_block = nullptr;
	llvm::BasicBlock* first_block = nullptr;

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
			condition = llvm::BasicBlock::Create(context, "", function);
			first_block = condition;
			builder.CreateBr(condition);
		}
		if(inst + 1 == instructions.size())
		{
			next_condition = tail_block;
		}
		else
		{
			next_condition = llvm::BasicBlock::Create(context, "", function);
		}
		op_block = llvm::BasicBlock::Create(context, "", function);
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
	builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), 0));

	for(auto& r: registers)
	{
		if(r.second->offset() != 0)
			create_closure(r.second.get(), context, module);
	}
}

void Program::create_closure(xerxzema::Register *reg, llvm::LLVMContext& context,
							 llvm::Module* module)
{
	std::vector<llvm::Type*> arg_types;
	arg_types.push_back(state_type->getPointerTo());

	if(reg->type()->name() != "unit")
		arg_types.push_back(reg->type()->type(context)->getPointerTo());

	auto closure_type = llvm::FunctionType::get(llvm::Type::getInt64Ty(context),
												arg_types, false);

	auto fn = llvm::Function::Create(closure_type,
									  llvm::GlobalValue::LinkageTypes::ExternalLinkage,
									  _name + "_closure" + reg->name(), module);
	llvm::Value* arg_ptr = nullptr;
	auto it = fn->arg_begin();

	auto state = &*it;
	it++;
	if(reg->type()->name() != "unit")
	{
		arg_ptr = &*it;
		it++;
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
		builder.CreateAtomicRMW(llvm::AtomicRMWInst::Or, ptr, mask,
								llvm::AtomicOrdering::AcquireRelease);

	}
	builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), 0));
}
};
