#include "Register.h"

namespace xerxzema
{

Register::Register(const std::string& name) : _name(name), _type(nullptr)
{

}

void Register::type(xerxzema::Type *t)
{
	_type = t;
}

};
