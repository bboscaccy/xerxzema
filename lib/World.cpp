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
	core->add_type("opaque", std::make_unique<Opaque>());
	core->add_instruction(create_def<Instruction>("nop", {"unit"}, {"unit"}));
	core->add_instruction(create_def<AddReal>("add", {"real", "real"}, {"real"}));
	core->add_instruction(create_def<SubReal>("sub", {"real", "real"}, {"real"}));
	core->add_instruction(create_def<MulReal>("mul", {"real", "real"}, {"real"}));
	core->add_instruction(create_def<DivReal>("div", {"real", "real"}, {"real"}));
	core->add_instruction(create_def<PowReal>("pow", {"real", "real"}, {"real"}));
	core->add_instruction(create_def<EqReal>("eq", {"real", "real"}, {"bool"}));
	core->add_instruction(create_def<NeReal>("ne", {"real", "real"}, {"bool"}));
	core->add_instruction(create_def<LtReal>("lt", {"real", "real"}, {"bool"}));
	core->add_instruction(create_def<LeReal>("le", {"real", "real"}, {"bool"}));
	core->add_instruction(create_def<GtReal>("gt", {"real", "real"}, {"bool"}));
	core->add_instruction(create_def<GeReal>("ge", {"real", "real"}, {"bool"}));
	core->add_instruction(create_def<Trace>("trace", {"real"}, {}));
	core->add_instruction(create_def<Merge>("merge", {"real", "real"}, {"real"}));
	core->add_instruction(create_def<Cond>("cond", {"bool", "real"}, {"real"}));
	core->add_instruction(create_def<When>("when", {"bool", "real"}, {"real"}));
	core->add_instruction(create_def<Delay>("delay", {"real"}, {"real"}));
	core->add_instruction(create_def<Bang>("bang", {}, {"real"}));
	core->add_instruction(create_def<Schedule>("schedule_absolute", {"int"}, {"unit"}));

	namespaces.emplace("core", std::move(core));

}

};
