#pragma once

#include <stdint.h>
#include <string>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Support/raw_ostream.h"

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

inline llvm::Value* const_int8(llvm::LLVMContext& context, uint64_t v)
{
	return llvm::ConstantInt::get(llvm::Type::getInt8Ty(context), v);
}

inline llvm::Value* const_int16(llvm::LLVMContext& context, uint64_t v)
{
	return llvm::ConstantInt::get(llvm::Type::getInt16Ty(context), v);
}

inline llvm::Value* const_int32(llvm::LLVMContext& context, uint64_t v)
{
	return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), v);
}

inline llvm::Value* const_int64(llvm::LLVMContext& context, uint64_t v)
{
	return llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), v);
}

inline std::string mangle(const std::string& name, const llvm::DataLayout& layout)
{
	std::string store;
	llvm::Mangler mangler;
	llvm::raw_string_ostream mangle_stream(store);
	mangler.getNameWithPrefix(mangle_stream, name, layout);
	return mangle_stream.str();
}


};
