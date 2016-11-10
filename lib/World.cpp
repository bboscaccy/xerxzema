#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"


#include "World.h"
#include "Type.h"

namespace xerxzema
{
World::World()
{
	create_core_namespace();
	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmParsers();
	llvm::InitializeAllAsmPrinters();
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
	core->add_type("atom", std::make_unique<Atom>());
	core->add_type("unit", std::make_unique<Unit>());
	core->add_type("buffer", std::make_unique<Buffer>());
	core->add_type("token", std::make_unique<Token>());
	namespaces.emplace("core", std::move(core));

}
};
