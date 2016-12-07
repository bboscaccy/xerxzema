#pragma once

#include <stdint.h>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"

namespace xerxzema
{

template<int N>
inline llvm::Value* const_int_value(llvm::LLVMContext& context, uint64_t v)
{
	return llvm::ConstantInt::get(context, llvm::APInt(N, v));
}

inline llvm::Value* const_int1(llvm::LLVMContext& context, uint64_t v)
{
	return llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), v);
}

llvm::Value* const_int8(llvm::LLVMContext& context, uint64_t v)
{
	return llvm::ConstantInt::get(llvm::Type::getInt8Ty(context), v);
}

llvm::Value* const_int16(llvm::LLVMContext& context, uint64_t v)
{
	return llvm::ConstantInt::get(llvm::Type::getInt16Ty(context), v);
}

llvm::Value* const_int32(llvm::LLVMContext& context, uint64_t v)
{
	return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), v);
}

llvm::Value* const_int64(llvm::LLVMContext& context, uint64_t v)
{
	return llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), v);
}
};
