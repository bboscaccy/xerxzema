#pragma once
#include <string>
#include "Type.h"

namespace xerxzema
{

class Register
{
public:
	Register(const std::string& name);
	inline Type* type() const { return _type; }
	void type(Type* t);

	inline bool is_inferred() const { return type() != nullptr; }
	inline const std::string& name() const { return _name; }

private:
	std::string _name;
	Type* _type;
};

};
