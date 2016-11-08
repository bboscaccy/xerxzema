#pragma once
#include <string>
#include <map>
#include <memory>
#include "Namespace.h"

namespace xerxzema
{
class World
{
public:
	World();
	Namespace* get_namespace(const std::string& name);
	std::vector<std::string> namespace_list() const;
private:
	void create_core_namespace();
	std::map<std::string, std::unique_ptr<Namespace>> namespaces;
};


};
