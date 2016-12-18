#include "Namespace.h"
#include "World.h"
#include "llvm/IR/IRBuilder.h"
#include "Diagnostics.h"

namespace xerxzema
{

Namespace::Namespace(World* w, const std::string& n) : _world(w), _name(n), parent(nullptr)
{
}

Namespace::Namespace(World* w, const std::string& n, Namespace* p) : _world(w), _name(n), parent(p)
{
}

Namespace::~Namespace()
{
}

void Namespace::codegen(llvm::Module *module, llvm::LLVMContext &context)
{
	//create required globals
	//TODO we don't want to just dump every possible global and extern
	//into every module.
	//load these on demand.
	//so we want extern_function(...
	//and extern_var(...
	scheduler = new llvm::GlobalVariable(*module,
										 llvm::Type::getVoidTy(context)->getPointerTo(),
										 false,
										 llvm::GlobalValue::LinkageTypes::ExternalLinkage,
										 nullptr, "xerxzema_scheduler");

	auto callback_type = llvm::FunctionType::get
		(llvm::Type::getVoidTy(context), {llvm::Type::getVoidTy(context)->getPointerTo()},
		 false);

	auto scheduler_run_type = llvm::FunctionType::get
		(llvm::Type::getVoidTy(context),
		 { llvm::Type::getVoidTy(context)->getPointerTo(),
				 callback_type->getPointerTo(),
				 llvm::Type::getVoidTy(context)->getPointerTo(),
				 llvm::Type::getInt64Ty(context)}, false);

	auto schedule_fn = llvm::Function::Create(scheduler_run_type,
											  llvm::Function::ExternalLinkage,
											  "xerxzema_schedule", module);

	for(auto& program: programs)
	{
		program.second->code_gen(module, context);
	}
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

Program* Namespace::get_default_program()
{
	return get_program("<default>");
}

bool Namespace::is_program(const std::string &name)
{
	return programs.find(name) != programs.end();
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

void Namespace::add_parameterized_type(const std::string &name,
									   std::unique_ptr<ParameterizedType> &&type)
{
	parameterized_types.emplace(name, std::move(type));
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

Type* Namespace::type(const std::string& name, const std::vector<Type*> params)
{
	if(parameterized_types.find(name) != parameterized_types.end())
	{
		auto instance = parameterized_types[name]->instantiate(params);
		if(parameterized_type_instances.find(instance->name()) ==
		   parameterized_type_instances.end())
		{
			auto ret_val = instance.get();
			parameterized_type_instances.emplace(ret_val->name(), std::move(instance));
			return ret_val;
		}
		else
		{
			return parameterized_type_instances[instance->name()].get();
		}
	}


	for(auto& ns : imports)
	{
		auto type = ns->type(name, params);
		if(type)
			return type;
	}
	return nullptr;
}

void Namespace::add_instruction(std::unique_ptr<InstructionDefinition> &&def)
{
	instructions[def->name()].push_back(std::move(def));
}

InstructionDefinition* Namespace::resolve_instruction(const std::string &name,
													  const std::vector<Type *> &inputs)
{
	auto it = instructions.find(name);
	if(it == instructions.end())
	{
		for(auto& ns : imports)
		{
			auto def = ns->resolve_instruction(name, inputs);
			if(def)
				return def;
		}
	}
	else
	{
		for(auto& def : it->second)
		{
			auto target_types = def->input_types(this);
			if(target_types.size() == inputs.size())
			{
				auto target = target_types.begin();
				bool valid = true;
				for(auto& type:inputs)
				{
					if(*target != type)
					{
						valid = false;
						break;
					}
					target++;
				}
				if(valid)
					return def.get();
			}
		}
	}
	return nullptr;
}
};
