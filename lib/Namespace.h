#pragma once
#include <string>
#include <map>
#include <memory>
#include <vector>
#include "Type.h"
#include "Program.h"
#include "Instruction.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"


namespace xerxzema
{
class World;
class InstructionDefinition;
class ExternalDefinition;
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
	void add_type_alias(const std::string& name, Type* type);
	void add_parameterized_type(const std::string& name, std::unique_ptr<ParameterizedType>&& type);
	Type* type(const std::string& name);
	Type* type(const std::string& name, const std::vector<Type*> params);
	inline bool is_type(const std::string& name)
	{
		return type(name) != nullptr;
	}
	Program* get_program(const std::string& name);
	Program* get_default_program();
	bool is_program(const std::string& name);

	void add_instruction(std::unique_ptr<InstructionDefinition>&& def);
	InstructionDefinition* resolve_instruction(const std::string& name,
											   const std::vector<Type*>& inputs);

	void add_external_mapping(ExternalDefinition* def, const std::string& local_name="");
	llvm::GlobalVariable* get_external_variable(const std::string& name, llvm::Module* module,
												llvm::LLVMContext& context);
	llvm::Function* get_external_function(const std::string& name, llvm::Module* module,
										  llvm::LLVMContext& context);

	inline std::vector<Program*> get_programs()
	{
		std::vector<Program*> p;
		p.reserve(programs.size());
		for(auto& it: programs)
		{
			p.push_back(it.second.get());
		}
		return p;
	}

private:
	std::map<std::string, std::unique_ptr<Namespace>> namespaces;
	std::map<std::string, std::unique_ptr<Program>> programs;
	std::vector<Namespace*> imports;
	std::map<std::string, std::unique_ptr<Type>> types;
	std::map<std::string, Type*> type_aliases;
	std::map<std::string, std::unique_ptr<ParameterizedType>> parameterized_types;
	std::map<std::string, std::unique_ptr<ParameterizedType>> parameterized_type_instances;
	std::map<std::string, std::vector<std::unique_ptr<InstructionDefinition>>> instructions;
	std::map<std::string, ExternalDefinition*> externals;
	World* _world;
	Namespace* parent;
	std::string _name;
	llvm::Value* scheduler;
};



class InstructionDefinition
{
public:
	virtual ~InstructionDefinition() = default;
	virtual std::unique_ptr<Instruction> create(const std::vector<Type *> &inputs,
												const std::vector<Type *> &outputs) = 0;
	//this is only used for the default match implementation
	virtual std::vector<Type*> input_types(Namespace* parent);
	virtual std::vector<Type*> output_types(const std::vector<Type*>& inputs, Namespace* parent) = 0;
	virtual std::string name() = 0;
	virtual bool match(const std::vector<Type*>& inputs, Namespace* parent);
};

template<class T>
class BasicDefinition : public InstructionDefinition
{
public:
	BasicDefinition(const std::string& name, const std::vector<std::string>& inputs,
					const std::vector<std::string>& outputs) : ins(inputs), outs(outputs),
															   _name(name)
	{
	}

	std::unique_ptr<Instruction> create(const std::vector<Type *> &inputs,
										const std::vector<Type *> &outputs)
	{ return std::make_unique<T>(); }

	std::string name() { return _name; }

	std::vector<Type*> input_types(Namespace* parent)
	{
		std::vector<Type*> results;
		for(auto& s: ins)
		{
			results.push_back(parent->type(s));
		}
		return results;
	}
	std::vector<Type*> output_types(const std::vector<Type*>& inputs, Namespace* parent)
	{
		std::vector<Type*> results;
		for(auto& s: outs)
		{
			results.push_back(parent->type(s));
		}
		return results;
	}

private:
	std::vector<std::string> ins;
	std::vector<std::string> outs;
	std::string _name;

};

class ProgramCallDefinition : public InstructionDefinition
{
public:
	ProgramCallDefinition(Program* target);
	std::unique_ptr<Instruction> create(const std::vector<Type *> &inputs,
										const std::vector<Type *> &outputs);
	std::vector<Type*> input_types(Namespace* parent);
	std::vector<Type*> output_types(const std::vector<Type*>& inputs, Namespace* parent);
	std::string name();

private:
	Program* target;

};

class ArrayBuilderDefinition : public InstructionDefinition
{
public:
	std::unique_ptr<Instruction> create(const std::vector<Type *> &inputs,
										const std::vector<Type *> &outputs);
	std::vector<Type*> output_types(const std::vector<Type*>& inputs, Namespace* parent);
	bool match(const std::vector<Type*>& inputs, Namespace* parent);
	inline std::string name() { return "array"; }
};

class WhenDefinition : public InstructionDefinition
{
public:
	std::unique_ptr<Instruction> create(const std::vector<Type *> &inputs,
										const std::vector<Type *> &outputs);
	std::vector<Type*> output_types(const std::vector<Type*>& inputs, Namespace* parent);
	bool match(const std::vector<Type*>& inputs, Namespace* parent);
	inline std::string name() { return "when"; }
};

class CondDefinition : public InstructionDefinition
{
public:
	std::unique_ptr<Instruction> create(const std::vector<Type *> &inputs,
										const std::vector<Type *> &outputs);
	std::vector<Type*> output_types(const std::vector<Type*>& inputs, Namespace* parent);
	bool match(const std::vector<Type*>& inputs, Namespace* parent);
	inline std::string name() { return "cond"; }
};

class MergeDefinition : public InstructionDefinition
{
public:
	std::unique_ptr<Instruction> create(const std::vector<Type *> &inputs,
										const std::vector<Type *> &outputs);
	std::vector<Type*> output_types(const std::vector<Type*>& inputs, Namespace* parent);
	bool match(const std::vector<Type*>& inputs, Namespace* parent);
	inline std::string name() { return "merge"; }
};

class SeqDefinition : public InstructionDefinition
{
public:
	std::unique_ptr<Instruction> create(const std::vector<Type *> &inputs,
										const std::vector<Type *> &outputs);
	std::vector<Type*> output_types(const std::vector<Type*>& inputs, Namespace* parent);
	bool match(const std::vector<Type*>& inputs, Namespace* parent);
	inline std::string name() { return "seq"; }
};

class DelayDefinition : public InstructionDefinition
{
public:
	std::unique_ptr<Instruction> create(const std::vector<Type *> &inputs,
										const std::vector<Type *> &outputs);
	std::vector<Type*> output_types(const std::vector<Type*>& inputs, Namespace* parent);
	bool match(const std::vector<Type*>& inputs, Namespace* parent);
	inline std::string name() { return "delay"; }
};


template<class T>
std::unique_ptr<BasicDefinition<T>> create_def(const std::string& name,
											   const std::vector<std::string>& inputs,
											   const std::vector<std::string>& outputs)
{
	return std::make_unique<BasicDefinition<T>>(name, inputs, outputs);
}
};
