#include "Type.h"

namespace xerxzema
{

Type::Type()
{

}

std::string Bool::name()
{
	return "bool";
}

llvm::Type* Bool::type(llvm::LLVMContext &context)
{
	return llvm::Type::getInt1Ty(context);
}

void Bool::init(llvm::LLVMContext &context, llvm::IRBuilder<> &builder, llvm::Value *value)
{
	builder.CreateStore(llvm::ConstantInt::get(type(context), 0), value);
}

llvm::Type* Unit::type(llvm::LLVMContext &context)
{
	return llvm::Type::getVoidTy(context);
}

std::string Unit::name()
{
	return "unit";
}

void Unit::init(llvm::LLVMContext &context, llvm::IRBuilder<> &builder, llvm::Value *value)
{

}

llvm::Type* Real::type(llvm::LLVMContext &context)
{
	return llvm::Type::getDoubleTy(context);
}

void Real::init(llvm::LLVMContext &context, llvm::IRBuilder<> &builder, llvm::Value *value)
{
	builder.CreateStore(llvm::ConstantFP::get(type(context), 0), value);
}

std::string Real::name()
{
	return "real";
}

llvm::Type* Int::type(llvm::LLVMContext &context)
{
	return llvm::Type::getInt64Ty(context);
}

std::string Int::name()
{
	return "int";
}

void Int::init(llvm::LLVMContext &context, llvm::IRBuilder<> &builder, llvm::Value *value)
{
	builder.CreateStore(llvm::ConstantInt::get(type(context), 0), value);
}


llvm::Type* Token::type(llvm::LLVMContext &context)
{
	return llvm::Type::getVoidTy(context)->getPointerTo();
}

std::string Token::name()
{
	return "token";
}

void Token::init(llvm::LLVMContext &context, llvm::IRBuilder<> &builder, llvm::Value *value)
{
	builder.CreateStore(llvm::ConstantInt::get(type(context), 0), value);
}



};
