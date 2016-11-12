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
		_inputs.push_back(reg);
	}

	inline std::vector<Register*>& inputs()
	{
		return _inputs;
	}

	inline void output(Register* reg)
	{
		_outputs.push_back(reg);
	}

	inline std::vector<Register*>& outputs()
	{
		return _outputs;
	}

	inline uint32_t offset() { return _offset; }
	inline void offset(uint32_t o)
	{
		_offset = o;
	}

protected:
	std::vector<Register*> _inputs;
	std::vector<Register*> _outputs;
	uint32_t _offset;
};

DEF_INST(Add)

};
