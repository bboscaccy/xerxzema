#pragma once
#include <string>
#include <stdint.h>
#include "Type.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"

namespace xerxzema
{
class Instruction;
struct DeferredInstruction;

struct ActivationMask
{
	Instruction* instruction;
	uint16_t value;
};


class Register
{
public:
	Register(const std::string& name);
	inline Type* type() const { return _type; }
	void type(Type* t);
	void offset(uint32_t o);
	inline uint32_t offset() { return state_offset; }

	inline bool is_inferred() const { return type() != nullptr; }
	inline const std::string& name() const { return _name; }

	inline void value(llvm::Value* val) { _value = val; }

	void activation(Instruction* instruction, uint16_t value);
	llvm::Value* fetch_value(llvm::LLVMContext& context,
							 llvm::IRBuilder<> &builder);
	llvm::Value* fetch_value_raw(llvm::LLVMContext& context,
								 llvm::IRBuilder<> &builder);
	void do_activations(llvm::LLVMContext& context,
						llvm::IRBuilder<> &builder);


	std::vector<DeferredInstruction*> deffered;
	std::vector<ActivationMask> activations;
private:
	llvm::Value* _value;
	std::string _name;
	Type* _type;
	uint32_t state_offset;
};

};
