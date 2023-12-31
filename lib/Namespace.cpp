#include "Namespace.h"
#include "World.h"
#include "llvm/IR/IRBuilder.h"
#include "Diagnostics.h"
#include "Instruction.h"

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

llvm::Function* Namespace::get_external_function(const std::string &name, llvm::Module *module,
												 llvm::LLVMContext& context)
{
	auto it = externals.find(name);
	if(it == externals.end())
	{
		for(auto& ns:imports)
		{
			auto res = ns->get_external_function(name, module, context);
			if(res)
				return res;
		}
		emit_error("cannot find external function: " + name);
		return nullptr;
	}
	else
	{
		return it->second->get_call(module, context);
	}
}

llvm::GlobalVariable* Namespace::get_external_variable(const std::string &name, llvm::Module *module,
													   llvm::LLVMContext& context)
{
	auto it = externals.find(name);
	if(it == externals.end())
	{
		for(auto& ns:imports)
		{
			auto res = ns->get_external_variable(name, module, context);
			if(res)
				return res;
		}
		return nullptr;
	}
	else
	{
		return it->second->get_variable(module, context);
	}
}


void Namespace::add_external_mapping(xerxzema::ExternalDefinition *def, const std::string& name)
{
	if(name == "")
	{
		externals[def->short_name()] = def;
	}
	else
	{
		externals[name] = def;
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
	return get_program(_name + ".default");
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

void Namespace::add_type_alias(const std::string &name, xerxzema::Type *type)
{
	type_aliases[name] = type;
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

	if(type_aliases.find(name) != type_aliases.end())
		return type_aliases[name];

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
			if(def->match(inputs, this))
			   return def.get();
		}
	}
	return nullptr;
}


bool InstructionDefinition::match(const std::vector<Type *> &inputs, Namespace* parent)
{
	auto target_types = input_types(parent);
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
			return true;
	}
	return false;
}

std::vector<Type*> InstructionDefinition::input_types(Namespace* parent)
{
	return std::vector<Type*>();
}

ProgramCallDefinition::ProgramCallDefinition(Program* target): target(target) {}
std::string ProgramCallDefinition::name()
{
	return target->program_name();
}

std::unique_ptr<Instruction> ProgramCallDefinition::create(const std::vector<Type *> &inputs,
														   const std::vector<Type *> &outputs)
{
	return std::make_unique<ProgramDirectCall>(target);
}

std::vector<Type*> ProgramCallDefinition::input_types(Namespace* parent)
{
	std::vector<Type*> types;
	for(auto& r:target->input_registers())
	{
		types.push_back(r->type());
	}
	return types;
}

std::vector<Type*> ProgramCallDefinition::output_types(const std::vector<Type*>& inputs,
													   Namespace* parent)
{
	std::vector<Type*> types;
	for(auto& r:target->output_registers())
	{
		types.push_back(r->type());
	}
	return types;
}


bool ArrayBuilderDefinition::match(const std::vector<Type *> &inputs, xerxzema::Namespace *parent)
{
	auto first_type = inputs[0];
	for(auto& next_type:inputs)
	{
		if(next_type != first_type)
			return false;
	}
	return true;
}

std::vector<Type*> ArrayBuilderDefinition::output_types(const std::vector<Type *> &inputs,
														xerxzema::Namespace *parent)
{
	return std::vector<Type*>{parent->type("array", {inputs[0]})};
}

std::unique_ptr<Instruction> ArrayBuilderDefinition::create(const std::vector<Type *> &inputs,
															const std::vector<Type *> &outputs)
{
	return std::make_unique<ArrayBuilder>(inputs[0]);
}


bool WhenDefinition::match(const std::vector<Type *> &inputs, xerxzema::Namespace *parent)
{

	if(inputs.size() != 2)
		return false;

	auto first_type = inputs[0];
	if(first_type->name() != "bool")
		return false;

	return true;
}

std::vector<Type*> WhenDefinition::output_types(const std::vector<Type *> &inputs,
												xerxzema::Namespace *parent)
{
	return std::vector<Type*>{inputs[1]};
}

std::unique_ptr<Instruction> WhenDefinition::create(const std::vector<Type *> &inputs,
													const std::vector<Type *> &outputs)
{
	return std::make_unique<When>();
}

bool CondDefinition::match(const std::vector<Type *> &inputs, xerxzema::Namespace *parent)
{

	if(inputs.size() != 2)
		return false;

	auto first_type = inputs[0];
	if(first_type->name() != "bool")
		return false;

	return true;
}

std::vector<Type*> CondDefinition::output_types(const std::vector<Type *> &inputs,
												xerxzema::Namespace *parent)
{
	return std::vector<Type*>{inputs[1]};
}

std::unique_ptr<Instruction> CondDefinition::create(const std::vector<Type *> &inputs,
													const std::vector<Type *> &outputs)
{
	return std::make_unique<Cond>();
}


bool DelayDefinition::match(const std::vector<Type *> &inputs, xerxzema::Namespace *parent)
{

	if(inputs.size() != 1)
		return false;
	return true;
}

std::vector<Type*> DelayDefinition::output_types(const std::vector<Type *> &inputs,
												xerxzema::Namespace *parent)
{
	return std::vector<Type*>{inputs[0]};
}

std::unique_ptr<Instruction> DelayDefinition::create(const std::vector<Type *> &inputs,
													const std::vector<Type *> &outputs)
{
	return std::make_unique<Delay>();
}


bool MergeDefinition::match(const std::vector<Type *> &inputs, xerxzema::Namespace *parent)
{
	auto first_type = inputs[0];
	for(auto& next_type:inputs)
	{
		if(next_type != first_type)
			return false;
	}
	return true;
}

std::vector<Type*> MergeDefinition::output_types(const std::vector<Type *> &inputs,
														xerxzema::Namespace *parent)
{
	return std::vector<Type*>{inputs[0]};
}

std::unique_ptr<Instruction> MergeDefinition::create(const std::vector<Type *> &inputs,
															const std::vector<Type *> &outputs)
{
	return std::make_unique<Merge>();
}

bool SeqDefinition::match(const std::vector<Type *> &inputs, xerxzema::Namespace *parent)
{
	auto first_type = inputs[0];
	for(auto& next_type:inputs)
	{
		if(next_type != first_type)
			return false;
	}
	return true;
}

std::vector<Type*> SeqDefinition::output_types(const std::vector<Type *> &inputs,
														xerxzema::Namespace *parent)
{
	return std::vector<Type*>{inputs[0]};
}

std::unique_ptr<Instruction> SeqDefinition::create(const std::vector<Type *> &inputs,
															const std::vector<Type *> &outputs)
{
	return std::make_unique<Seq>();
}



};
