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

llvm::Type* Unit::type(llvm::LLVMContext &context)
{
	return llvm::Type::getVoidTy(context);
}

std::string Unit::name()
{
	return "unit";
}

llvm::Type* Real::type(llvm::LLVMContext &context)
{
	return llvm::Type::getDoubleTy(context);
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

llvm::Type* Token::type(llvm::LLVMContext &context)
{
	return llvm::Type::getVoidTy(context)->getPointerTo();
}

std::string Token::name()
{
	return "token";
}

};
