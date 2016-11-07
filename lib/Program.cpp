#include "Program.h"

namespace xerxzema
{
Program::Program(const std::string& name) : _name(name)
{

}

void Program::add_input(const std::string &name, xerxzema::Type *type)
{
	auto r = std::make_unique<Register>(name);
	r->type(type);
	registers.emplace(name, std::move(r));
}

void Program::add_output(const std::string &name, xerxzema::Type *type)
{
	auto r = std::make_unique<Register>(name);
	r->type(type);
	registers.emplace(name, std::move(r));
}

};
