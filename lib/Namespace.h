#pragma once
#include <string>
#include <map>
#include <memory>
#include <vector>
#include "Type.h"
#include "Program.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"


namespace xerxzema
{
class World;
class Namespace
{
public:
	Namespace(World* world, const std::string& name);
	Namespace(World* world, const std::string& name, Namespace* parent);
	~Namespace();
	Namespace* get_namespace(const std::string& name);
	std::vector<std::string> namespace_list() const;
	inline World* world() const { return _world; }
	inline const std::string& name() const { return _name; }
	std::string full_name() const;
	void import(Namespace* ns);
	void add_type(const std::string& name, std::unique_ptr<Type>&& type);
	Type* type(const std::string& name);
	inline bool is_type(const std::string& name)
	{
		return type(name) != nullptr;
	}
	Program* get_program(const std::string& name);
	bool is_program(const std::string& name);
	void codegen(llvm::Module* module, llvm::LLVMContext& context);

private:
	std::map<std::string, std::unique_ptr<Namespace>> namespaces;
	std::map<std::string, std::unique_ptr<Program>> programs;
	std::vector<Namespace*> imports;
	std::map<std::string, std::unique_ptr<Type>> types;
	World* _world;
	Namespace* parent;
	std::string _name;
};
};
