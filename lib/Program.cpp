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
	if(!check_instruction(name, inputs, outputs))
	{
		auto def = std::make_unique<DeferredInstruction>();
		def->name = name;
		def->inputs = inputs;
		def->outputs = outputs;
		def->solved = false;

		for(auto& r: inputs)
		{
			reg(r)->deffered.push_back(def.get());
		}

		deferred.push_back(std::move(def));
	}

}

bool Program::check_instruction(const std::string &name,
								const std::vector<std::string> &inputs,
								const std::vector<std::string> &outputs)
{
	bool resolved = true;
	std::vector<Type*> input_types;
	for(auto reg_name: inputs)
	{
		if(!reg(reg_name)->type())
		{
			resolved = false;
			break;
		}
		else
		{
			input_types.push_back(reg(reg_name)->type());
		}
	}
	if(resolved)
	{
		auto def = parent->resolve_instruction(name, input_types);
		if(def)
		{
			auto output_types = def->output_types(parent);
			if(output_types.size() == outputs.size())
			{
				auto it = outputs.begin();
				for(auto& type: output_types)
				{
					auto out_reg = reg(*it);
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
								if(check_instruction(retry->name, retry->inputs, retry->outputs))
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
					inst->input(reg(n));
				}
				for(auto& n:outputs)
				{
					inst->output(reg(n));
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
	data_types.push_back(llvm::Type::getInt64Ty(context)); //frame counter
	data_types.push_back(llvm::Type::getInt1Ty(context)); //state value
	data_types.push_back(llvm::Type::getInt64Ty(context)); //offset table
	int i = 3;
	for(auto r: inputs)
	{
		data_types.push_back(r->type()->type(context));
		r->offset(i);
		i++;
	}
	for(auto r: outputs)
	{
		data_types.push_back(r->type()->type(context));
		r->offset(i);
		i++;
	}
	for(auto r: locals)
	{
		//data_types.push_back(r->type()->type(context));
		//r->offset(i);
		//i++;
	}

	auto state_type = llvm::StructType::create(context, data_types, _name + "_data");

	std::vector<llvm::Type*> arg_types;
	arg_types.push_back(state_type->getPointerTo());

	return llvm::FunctionType::get(llvm::Type::getInt64Ty(context), arg_types, false);
}


void Program::allocate_registers(llvm::LLVMContext& context, llvm::IRBuilder<>& builder,
								 llvm::Value* state)
{
	for(auto r: inputs)
	{
		auto ptr = builder.CreateStructGEP(state_type, state, r->offset());
		r->value(ptr);
	}

	for(auto r: outputs)
	{
		auto ptr = builder.CreateStructGEP(state_type, state, r->offset());
		r->value(ptr);
	}

	for(auto r: locals)
	{
		r->value(builder.CreateAlloca(r->type()->type(context), nullptr, r->name()));
	}
	for(auto& i: instructions)
	{
		i->value(builder.CreateAlloca(llvm::Type::getInt16Ty(context)));
	}
	activation_counter = builder.CreateAlloca(llvm::Type::getInt64Ty(context), nullptr, "counter");
}

void Program::code_gen(llvm::Module *module, llvm::LLVMContext &context)
{
	auto ftype = function_type(context);
	function = llvm::Function::Create(ftype,
									 llvm::GlobalValue::LinkageTypes::ExternalLinkage,
									 _name , module);

	llvm::IRBuilder<> builder(context);
	auto state = &*function->arg_begin();
	auto head_block = llvm::BasicBlock::Create(context, "head", function);
	auto tail_block = llvm::BasicBlock::Create(context, "tail", function);
	builder.SetInsertPoint(head_block);
	allocate_registers(context, builder, state);
	reg("head")->do_activations(context, builder, state_type, state);

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
		(*it)->generate_check(context, builder, state_type, state,condition,op_block,next_condition);
		builder.SetInsertPoint(op_block);
		(*it)->generate_operation(context, builder, state_type, state);
		(*it)->generate_prolouge(context, builder, state_type, activation_counter, next_condition);
		inst++;
		it++;
	}

	builder.SetInsertPoint(tail_block);
	auto reenter_block = llvm::BasicBlock::Create(context, "reenter", function);
	auto exit_block = llvm::BasicBlock::Create(context, "exit", function);
	auto counter_value = builder.CreateLoad(activation_counter);
	auto reenter = builder.CreateICmp(llvm::CmpInst::Predicate::ICMP_EQ, counter_value,
										llvm::ConstantInt::get(context, llvm::APInt(64, 0)));
	builder.CreateCondBr(reenter, exit_block, reenter_block);

	builder.SetInsertPoint(reenter_block);
	builder.CreateStore(llvm::ConstantInt::get(context, llvm::APInt(64, 0)), activation_counter);
	builder.CreateBr(first_block);

	builder.SetInsertPoint(exit_block);
	builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), 0));
}

};
