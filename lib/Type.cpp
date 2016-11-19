#include "Type.h"

namespace xerxzema
{

Type::Type()
{

}

void Type::move(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
				llvm::Value *dst_ptr, llvm::Value *src_ptr)
{
	auto v = builder.CreateLoad(src_ptr);
	builder.CreateStore(v, dst_ptr);
}

void Type::copy(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
				llvm::Value *dst_ptr, llvm::Value *src_ptr)
{
	auto v = builder.CreateLoad(src_ptr);
	builder.CreateStore(v, dst_ptr);
}

void Type::destroy(llvm::LLVMContext &context, llvm::IRBuilder<> &builder, llvm::Value *v)
{

}

void ParameterizedType::instantiate(const std::vector<Type *> &params)
{
	type_params = params;
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


std::string Array::name()
{
	return "array@" + type_params[0]->name() + "@";
}

llvm::Type* Array::type(llvm::LLVMContext& context)
{
	std::vector<llvm::Type*> arg_types;
	arg_types.push_back(type_params[0]->type(context));
	arg_types.push_back(llvm::Type::getInt64Ty(context));
	arg_types.push_back(llvm::Type::getInt64Ty(context));
	return llvm::StructType::create(context, arg_types, name());
}

void Array::init(llvm::LLVMContext &context, llvm::IRBuilder<> &builder, llvm::Value *val)
{
	auto sz = llvm::ConstantExpr::getSizeOf(type(context));
	builder.CreateMemSet(val, llvm::ConstantInt::get(llvm::Type::getInt8Ty(context), 0),
						 sz, 0);

}

};
