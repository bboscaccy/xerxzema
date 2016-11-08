#pragma once
#include <string>
#include <map>
#include <memory>
#include <vector>

namespace xerxzema
{
class World;
class Namespace
{
public:
	Namespace(World* world, const std::string& name);
	Namespace(World* world, const std::string& name, Namespace* parent);
	Namespace* get_namespace(const std::string& name);
	std::vector<std::string> namespace_list() const;
	inline World* world() const { return _world; }
	inline const std::string& name() const { return _name; }

private:
	std::map<std::string, std::unique_ptr<Namespace>> namespaces;
	World* _world;
	Namespace* parent;
	std::string _name;
};
};
