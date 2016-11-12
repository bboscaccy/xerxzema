#pragma once
#include <string>
#include <stdint.h>
#include "Type.h"

namespace xerxzema
{
class Instruction;

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

	inline bool is_inferred() const { return type() != nullptr; }
	inline const std::string& name() const { return _name; }

	void activation(Instruction* instruction, uint16_t value);

private:
	std::string _name;
	Type* _type;
	std::vector<ActivationMask> activations;
};

};
