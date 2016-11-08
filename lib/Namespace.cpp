#include "Namespace.h"
#include "World.h"

namespace xerxzema
{

Namespace::Namespace(World* w, const std::string& n) : _world(w), _name(n), parent(nullptr)
{
}

Namespace::Namespace(World* w, const std::string& n, Namespace* p) : _world(w), _name(n), parent(p)
{
}

Namespace* Namespace::get_namespace(const std::string& name)
{
	auto it = namespaces.find(name);
	if(it != namespaces.end())
		return it->second.get();
	namespaces.emplace(name, std::make_unique<Namespace>(_world, name, this));
	return namespaces[name].get();
}

Program* Namespace::get_program(const std::string &name)
{
	auto it = programs.find(name);
	if(it != programs.end())
		return it->second.get();
	programs.emplace(name, std::make_unique<Program>(this, name));
	return programs[name].get();
}

std::vector<std::string> Namespace::namespace_list() const
{
	std::vector<std::string> names;
	for(const auto& itm : namespaces)
	{
		names.push_back(itm.first);
	}
	return names;
}

std::string Namespace::full_name() const
{
	if(parent)
		return parent->full_name() + "." + _name;
	return _name;
}

void Namespace::import(xerxzema::Namespace *ns)
{
	imports.push_back(ns);
}

void Namespace::add_type(const std::string &name, std::unique_ptr<Type> &&type)
{
	types.emplace(name, std::move(type));
}

Type* Namespace::type(const std::string& name)
{
	if(types.find(name) != types.end())
		return types[name].get();

	for(auto& ns : imports)
	{
		auto type = ns->type(name);
		if(type)
			return type;
	}
	return nullptr;
}

};
