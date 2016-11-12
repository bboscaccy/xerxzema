#include <string>
#include <vector>
#include "Type.h"

namespace xerxzema
{

class Register;

#define DEF_INST(x) class x : public Instruction { \
	std::vector<Type*> input_types();  \
	std::vector<Type*> output_types(); };

#define DEF_INST_VAR(x) class x : public Instruction { \
	std::vector<Type*> input_types(); \
	std::vector<Type*> output_types(); \
	inline bool has_variadic_output() { return true; } };\


class Instruction
{
public:
	Instruction();
	virtual std::vector<Type*> input_types();
	virtual std::vector<Type*> output_types();
	virtual bool has_variadic_output();

	inline void input(Register* reg)
	{
		inputs.push_back(reg);
	}

	inline std::vector<Register*>& get_inputs()
	{
		return inputs;
	}

	inline void output(Register* reg)
	{
		outputs.push_back(reg);
	}

	inline std::vector<Register*>& get_outputs()
	{
		return outputs;
	}

protected:
	std::vector<Register*> inputs;
	std::vector<Register*> outputs;
};

DEF_INST(Add)

};

