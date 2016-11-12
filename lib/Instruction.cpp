#include "Instruction.h"

namespace xerxzema
{

Instruction::Instruction()
{

}

bool Instruction::has_variadic_output()
{
	return false;
}

std::vector<Type*> Instruction::input_types()
{
	return std::vector<Type*>();
}

std::vector<Type*> Instruction::output_types()
{
	return std::vector<Type*>();
}

};
