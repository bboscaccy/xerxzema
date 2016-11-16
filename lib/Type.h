#pragma once
#include <string>

#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"

namespace xerxzema
{

class Type
{
public:
	Type();
	virtual std::string name() = 0;
	virtual llvm::Type* type(llvm::LLVMContext& context) = 0;
	virtual void init(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Value* val) = 0;
};

#define DECL_TYPE(X) class X : public Type { \
public:										 \
 std::string name();						 \
 llvm::Type* type(llvm::LLVMContext& context); \
 void init(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Value* value);	};

DECL_TYPE(Bool)
DECL_TYPE(Unit)
DECL_TYPE(Real)
DECL_TYPE(Int)
DECL_TYPE(Token)

};
