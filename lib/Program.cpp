#include "Program.h"
#include "llvm/IR/IRBuilder.h"

namespace xerxzema
{
Program::Program(Namespace* p, const std::string& name) : parent(p), _name(name)
{

}

void Program::add_input(const std::string &name, xerxzema::Type *type)
{
	auto r = std::make_unique<Register>(name);
	r->type(type);
	registers.emplace(name, std::move(r));
}

void Program::add_output(const std::string &name, xerxzema::Type *type)
{
	auto r = std::make_unique<Register>(name);
	r->type(type);
	registers.emplace(name, std::move(r));
}

void Program::code_gen(llvm::Module *module, llvm::LLVMContext &context)
{
	std::vector<llvm::Type*> data_types;
	data_types.push_back(llvm::Type::getInt64Ty(context)); //frame counter
	data_types.push_back(llvm::Type::getInt1Ty(context)); //state value
	data_types.push_back(llvm::Type::getInt64Ty(context)); //offset table
	//inputs
	//outputs
	//locals
	auto program_data = llvm::StructType::create(context, data_types, _name + "_data");

	std::vector<llvm::Type*> arg_types;
	arg_types.push_back(program_data->getPointerTo());

	auto function_type = llvm::FunctionType::get(llvm::Type::getInt64Ty(context),
												 arg_types, false);
	auto fn = llvm::Function::Create(function_type,
									 llvm::GlobalValue::LinkageTypes::ExternalLinkage,
									 _name, module);

	llvm::IRBuilder<> builder(context);
	auto block = llvm::BasicBlock::Create(context, "entry", fn);
	builder.SetInsertPoint(block);
	builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), 0));
}

};
