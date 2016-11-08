#include "World.h"

namespace xerxzema
{
World::World()
{

}

Namespace* World::get_namespace(const std::string& name)
{
	auto it = namespaces.find(name);
	if(it != namespaces.end())
		return it->second.get();
	namespaces.emplace(name, std::make_unique<Namespace>(this, name));
	return namespaces[name].get();
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
};
