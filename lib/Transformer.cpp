#include "Transformer.h"
#include <algorithm>
#include <map>
#include "LLVMUtils.h"
#include <iostream>

namespace xerxzema
{

Transformer::Transformer(Program* prev, Program* next): prev(prev), next(next)
{

}

void Transformer::find_new_registers()
{
	for(auto& reg_pair: next->register_listing())
	{
		auto it = prev->register_listing().find(reg_pair.first);
		if(it == prev->register_listing().end())
		{
			new_registers.push_back(reg_pair.second.get());
		}
	}
}

void Transformer::find_deleted_registers()
{
	for(auto& reg_pair: prev->register_listing())
	{
		auto it = next->register_listing().find(reg_pair.first);
		if(it == next->register_listing().end())
		{
			deleted_registers.push_back(reg_pair.second.get());
		}
	}
}

void Transformer::find_type_change_registers()
{
	for(auto& reg_pair: prev->register_listing())
	{
		auto it = next->register_listing().find(reg_pair.first);
		if(it != next->register_listing().end())
		{
			if(it->second->type() != reg_pair.second->type())
				type_change_registers.push_back(RegMapping{reg_pair.second.get(), it->second.get()});
			else
				reusable_registers.push_back(RegMapping{reg_pair.second.get(), it->second.get()});
		}
	}
}

void Transformer::parse_registers()
{
	find_new_registers();
	find_deleted_registers();
	find_type_change_registers();
}

void Transformer::parse_instructions()
{
	std::multimap<std::string, Instruction*> input_set;
	for(auto& inst: next->instruction_listing())
	{
		input_set.insert({inst->diff_description(), inst.get()});
	}
	for(auto& inst: prev->instruction_listing())
	{
		auto it = input_set.find(inst->diff_description());
		if(it == input_set.end())
			deleted_instructions.push_back(inst.get());
		else
		{
			reusable_instructions.push_back(InstructionMapping{inst.get(), it->second});
			input_set.erase(it);
		}
	}
	for(auto& it: input_set)
	{
		new_instructions.push_back(it.second);
	}
}

llvm::Function* Transformer::generate_transformer(llvm::LLVMContext& context)
{
	auto prev_type = prev->state_type_value();
	auto next_type = next->state_type_value();

	std::vector<llvm::Type*> arg_types;
	arg_types.push_back(prev_type->getPointerTo());
	arg_types.push_back(next_type->getPointerTo());

	auto transformer_type = llvm::FunctionType::get(llvm::Type::getInt64Ty(context)
													,arg_types, false);

	auto function = llvm::Function::Create(transformer_type,
										   llvm::GlobalValue::LinkageTypes::ExternalLinkage,
										   next->program_name() + "@transformer",
										   next->current_module());

	auto arg_it = function->arg_begin();

	auto prev_arg = &*arg_it++;
	auto next_arg = &*arg_it++;

	auto block = llvm::BasicBlock::Create(context, "entry", function);
	llvm::IRBuilder<> builder(context);

	builder.SetInsertPoint(block);

	for(auto mapping: reusable_registers)
	{
		if(mapping.prev->offset() > 0)
		{
			std::cout << mapping.prev->name() << std::endl;
			std::cout << mapping.prev->offset() << std::endl;
			auto prev_ptr = builder.CreateStructGEP(prev_type, prev_arg, mapping.prev->offset());
			auto next_ptr = builder.CreateStructGEP(next_type, next_arg, mapping.next->offset());
			mapping.next->type()->copy(context, builder, next_ptr, prev_ptr);
		}
	}

	builder.CreateRet(const_int64(context, 0));
	next->current_module()->dump();
	return function;
}

};
