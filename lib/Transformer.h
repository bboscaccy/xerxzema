#pragma once
#include <vector>
#include <string>

#include "Program.h"

namespace xerxzema
{

struct RegMapping
{
	Register* prev;
	Register* next;
};

class Transformer
{
public:
	Transformer(Program* prev, Program* next);

	void parse_registers();

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

};

};
