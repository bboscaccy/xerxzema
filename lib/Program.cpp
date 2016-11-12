#include "Program.h"
#include "llvm/IR/IRBuilder.h"

namespace xerxzema
{
Program::Program(Namespace* p, const std::string& name) : parent(p), _name(name)
{
	reg("head");
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
	return result;
}

void Program::instruction(std::unique_ptr<Instruction>&& inst)
{
	instructions.push_back(std::move(inst));
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
		data_types.push_back(r->type()->type(context));
		r->offset(i);
		i++;
	}

	auto state_type = llvm::StructType::create(context, data_types, _name + "_data");

	std::vector<llvm::Type*> arg_types;
	arg_types.push_back(state_type->getPointerTo());

	return llvm::FunctionType::get(llvm::Type::getInt64Ty(context), arg_types, false);
}

void Program::code_gen(llvm::Module *module, llvm::LLVMContext &context)
{
	auto ftype = function_type(context);
	frame_function = llvm::Function::Create(ftype,
									 llvm::GlobalValue::LinkageTypes::ExternalLinkage,
									 _name + "-frame", module);

	llvm::IRBuilder<> builder(context);
	auto state = &*frame_function->arg_begin();
	auto head_block = llvm::BasicBlock::Create(context, "head", frame_function);
	auto tail_block = llvm::BasicBlock::Create(context, "tail", frame_function);
	builder.SetInsertPoint(head_block);
	reg("head")->do_activations(context, builder, state_type, state);

	llvm::BasicBlock* next_condition = nullptr;
	llvm::BasicBlock* condition = nullptr;
	llvm::BasicBlock* op_block = nullptr;

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
			condition = llvm::BasicBlock::Create(context,
												 "inst_cond" + std::to_string(inst),
												 frame_function);
			builder.CreateBr(condition);
		}
		if(inst + 1 == instructions.size())
		{
			next_condition = tail_block;
		}
		else
		{
			next_condition = llvm::BasicBlock::Create(context,
													  "inst_cond" + std::to_string(inst + 1),
													  frame_function);
		}
		op_block = llvm::BasicBlock::Create(context,
											"inst_op" + std::to_string(inst),
											frame_function);
		(*it)->generate_check(context, builder, state_type, state,condition,op_block,next_condition);
		builder.SetInsertPoint(op_block);
		(*it)->generate_operation(context, builder, state_type, state);
		(*it)->generate_prolouge(context, builder, state_type, state, next_condition);
		inst++;
		it++;
	}

	builder.SetInsertPoint(tail_block);
	builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), 0));
}

};
