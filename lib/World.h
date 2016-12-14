#pragma once
#include <string>
#include <map>
#include <memory>
#include "Namespace.h"
#include "Jit.h"
#include "Scheduler.h"

namespace xerxzema
{
class World
{
public:
	World();
	Namespace* get_namespace(const std::string& name);
	std::vector<std::string> namespace_list() const;
	Jit* jit();
	inline Scheduler* scheduler() { return _scheduler.get(); }
private:
	void create_core_namespace();
	std::map<std::string, std::unique_ptr<Namespace>> namespaces;
	std::unique_ptr<Scheduler> _scheduler;
	std::unique_ptr<Jit> jit_instance;
};
};
