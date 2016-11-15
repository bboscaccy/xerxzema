#include "World.h"
#include "Type.h"
#include "Instruction.h"

namespace xerxzema
{
World::World()
{
	create_core_namespace();
}

std::unique_ptr<Jit> World::create_jit()
{
	return std::make_unique<Jit>(this);
}

Namespace* World::get_namespace(const std::string& name)
{
	auto it = namespaces.find(name);
	if(it != namespaces.end())
		return it->second.get();
	namespaces.emplace(name, std::make_unique<Namespace>(this, name));
	auto n = namespaces[name].get();
	n->import(namespaces["core"].get());
	return n;
}

std::vector<std::string> World::namespace_list() const
{
	std::vector<std::string> names;
	for(const auto& itm : namespaces)
	{
		names.push_back(itm.first);
	}
	return names;
}

void World::create_core_namespace()
{
	auto core = std::make_unique<Namespace>(this, "core");
	core->add_type("bool", std::make_unique<Bool>());
	core->add_type("int", std::make_unique<Int>());
	core->add_type("real", std::make_unique<Real>());
	core->add_type("unit", std::make_unique<Unit>());
	core->add_type("token", std::make_unique<Token>());
	core->add_instruction(create_def<Instruction>("nop", {"unit"}, {"unit"}));
	core->add_instruction(create_def<AddReal>("add", {"real", "real"}, {"real"}));
	namespaces.emplace("core", std::move(core));

}

};
