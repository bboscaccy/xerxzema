#include "Register.h"
#include "Instruction.h"

namespace xerxzema
{

Register::Register(const std::string& name) : _name(name), _type(nullptr)
{

}

void Register::type(xerxzema::Type *t)
{
	_type = t;
}

void Register::activation(xerxzema::Instruction *instruction, uint16_t value)
{
	activations.push_back({instruction, value});
}

void Register::offset(uint32_t o)
{
	state_offset = o;
}

llvm::Value* Register::fetch_value(llvm::LLVMContext& context,
								   llvm::IRBuilder<> &builder,
								   llvm::Type* state_type,
								   llvm::Value* state)
{
	std::vector<llvm::Value*> idx =
		{llvm::ConstantInt::get(context, llvm::APInt(64, 0)),
		 llvm::ConstantInt::get(context, llvm::APInt(32, state_offset))};
	auto ptr = builder.CreateGEP(state_type, state, idx);
	return builder.CreateLoad(ptr);
}

llvm::Value* Register::fetch_value_raw(llvm::LLVMContext& context,
									   llvm::IRBuilder<> &builder,
									   llvm::Type* state_type,
									   llvm::Value* state)
{
	std::vector<llvm::Value*> idx =
		{llvm::ConstantInt::get(context, llvm::APInt(64, 0)),
		 llvm::ConstantInt::get(context, llvm::APInt(32, state_offset))};
	return builder.CreateGEP(state_type, state, idx);
}

void Register::do_activations(llvm::LLVMContext &context,
							  llvm::IRBuilder<> &builder,
							  llvm::Type *state_type,
							  llvm::Value* state)
{

	for(auto& activate:activations)
	{
		auto mask = builder.CreateLoad(activate.instruction->value());
		auto update = builder.CreateOr(mask, llvm::APInt(16, activate.value));
		builder.CreateStore(update, activate.instruction->value());
	}

}
};
