#include "Type.h"

namespace xerxzema
{

Type::Type()
{

}

void Type::move(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
				Program* program, llvm::Value *dst_ptr, llvm::Value *src_ptr)
{
	auto v = builder.CreateLoad(src_ptr);
	builder.CreateStore(v, dst_ptr);
}

void Type::copy(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
				Program* program, llvm::Value *dst_ptr, llvm::Value *src_ptr)
{
	auto v = builder.CreateLoad(src_ptr);
	builder.CreateStore(v, dst_ptr);
}

void Type::destroy(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
				   Program* program, llvm::Value *v)
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


void Bool::init(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
				Program* program, llvm::Value *value)
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

void Unit::init(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
				Program* program, llvm::Value *value)
{

}

llvm::Type* Real::type(llvm::LLVMContext &context)
{
	return llvm::Type::getDoubleTy(context);
}

void Real::init(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
				Program* program, llvm::Value *value)
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

void Int::init(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
			   Program* program, llvm::Value *value)
{
	builder.CreateStore(llvm::ConstantInt::get(type(context), 0), value);
}

std::string Byte::name()
{
	return "byte";
}

llvm::Type* Byte::type(llvm::LLVMContext &context)
{
	return llvm::Type::getInt8Ty(context);
}

void Byte::init(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
				Program* program, llvm::Value *value)
{
	builder.CreateStore(llvm::ConstantInt::get(type(context), 0), value);
}



llvm::Type* Opaque::type(llvm::LLVMContext &context)
{
	return llvm::Type::getInt8PtrTy(context);
}

std::string Opaque::name()
{
	return "opaque";
}

void Opaque::init(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
				  Program* program, llvm::Value *value)
{
	builder.CreateStore(llvm::ConstantInt::get(type(context), 0), value);
}

Array::Array() : cached_type(nullptr)
{

}


std::unique_ptr<ParameterizedType> Array::instantiate(const std::vector<Type *> &params)
{
	auto v = std::make_unique<Array>();
	v->type_params = params;
	return std::move(v);
}

std::string Array::name()
{
	return "array." + type_params[0]->name();
}

llvm::Type* Array::type(llvm::LLVMContext& context)
{
	if(cached_type)
		return cached_type;

	std::vector<llvm::Type*> arg_types;
	arg_types.push_back(type_params[0]->type(context)->getPointerTo());
	arg_types.push_back(llvm::Type::getInt64Ty(context)); // number of elements
	arg_types.push_back(llvm::Type::getInt64Ty(context)); // size of allocation
	cached_type = llvm::StructType::create(context, arg_types, name());
	return cached_type;
}

void Array::init(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
				 Program* program, llvm::Value *val)
{
	auto sz = llvm::ConstantExpr::getSizeOf(type(context));
	builder.CreateMemSet(val, llvm::ConstantInt::get(llvm::Type::getInt8Ty(context), 0),
						 sz, 0);

}

void Array::destroy(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
					Program* program, llvm::Value *v)
{
	/* we need to be able to call externals here....
	   somehow?
	if(type_params[0]->is_trivial())
	{
		auto void_cast = builder.CreatePointerCast(v, llvm::Type::getInt8PtrTy(context));
		builder.CreateCall(deallocator, {void_cast});
		auto sz = llvm::ConstantExpr::getSizeOf(_outputs[0]->type()->type(context));
		builder.CreateMemSet(out_struct, builder.getInt8(0), sz, 0);
	}
	*/
}
};
