#pragma once
#include <vector>
#include <string>

#include "Program.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"

namespace xerxzema
{

struct RegMapping
{
	Register* prev;
	Register* next;
};

struct InstructionMapping
{
	Instruction* prev;
	Instruction* next;
};

class Transformer
{
public:
	Transformer(Program* prev, Program* next);

	void parse_registers();
	void parse_instructions();
	llvm::Function* generate_transformer(llvm::LLVMContext& context);

	inline const std::vector<Register*>& get_new_registers()
	{
		return new_registers;
	}

	inline const std::vector<Register*>& get_deleted_registers()
	{
		return deleted_registers;
	}

	inline const std::vector<RegMapping>& get_type_change_registers()
	{
		return type_change_registers;
	}

	inline const std::vector<RegMapping>& get_reusable_registers()
	{
		return reusable_registers;
	}

	inline const std::vector<Instruction*> get_new_instructions()
	{
		return new_instructions;
	}

	inline const std::vector<Instruction*> get_deleted_instructions()
	{
		return new_instructions;
	}

	inline const std::vector<InstructionMapping> get_reusable_instructions()
	{
		return reusable_instructions;
	}

private:
	void find_new_registers();
	void find_deleted_registers();
	void find_type_change_registers();
	Program* prev;
	Program* next;
	std::vector<Register*> new_registers;
	std::vector<Register*> deleted_registers;
	std::vector<RegMapping> type_change_registers;
	std::vector<RegMapping> reusable_registers;
	std::vector<Instruction*> new_instructions;
	std::vector<Instruction*> deleted_instructions;
	std::vector<InstructionMapping> reusable_instructions;

};

};
