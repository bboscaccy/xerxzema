#include "World.h"
#include "Type.h"
#include "Instruction.h"
#include "RT.h"
#include "Diagnostics.h"

namespace xerxzema
{
World::World() : _scheduler(std::make_unique<Scheduler>())
{
	create_core_namespace();
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmParser();
	llvm::InitializeNativeTargetAsmPrinter();
	llvm::InitializeNativeTargetDisassembler();
}

Jit* World::jit()
{
	if(!jit_instance)
		jit_instance = std::make_unique<Jit>(this);
	return jit_instance.get();
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

llvm::Function*	ExternalDefinition::get_call(llvm::Module *module, llvm::LLVMContext& context)
{
	auto fn = module->getFunction(symbol_name);
	if(fn)
		return fn;
	std::vector<llvm::Type*> call_args;
	for(auto& type:types)
	{
		call_args.push_back(type->type(context));
	}
	auto fn_type = llvm::FunctionType::get(result_type->type(context), call_args, var_arg);
	fn = llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage, symbol_name, module);
	fn->setCallingConv(llvm::CallingConv::C);
	return fn;
}

llvm::GlobalVariable* ExternalDefinition::get_variable(llvm::Module* module, llvm::LLVMContext& context)
{
	auto gv = module->getGlobalVariable(symbol_name);
	if(gv)
		return gv;

	return new llvm::GlobalVariable(*module, result_type->type(context), false,
									llvm::GlobalValue::LinkageTypes::ExternalLinkage,
									nullptr, symbol_name);
}

//In the background LLVM maintaines a static thread safe contianer for lookups in here.
//our symbol resolution will append a lib name as a prefix.
//that should let us have duplicate named functions/vars in different external libs
ExternalDefinition::ExternalDefinition(const std::string& name, const std::vector<Type*> types,
									   Type* result_type, const std::string& lib_name, void* address,
									   bool var_arg):
	types(types), result_type(result_type), address(address),
	var_arg(var_arg)
{
	std::string err_msg;
	if(lib_name == "")
	{
		lib = llvm::sys::DynamicLibrary::getPermanentLibrary(nullptr);
		symbol_name = "xerxzema:" + name;
	}
	else
	{
		lib = llvm::sys::DynamicLibrary::getPermanentLibrary(lib_name.c_str(), &err_msg);
		symbol_name = lib_name + ":" + name;
		if(!lib.isValid())
		{
			emit_error(err_msg);
		}
	}
}

llvm::RuntimeDyld::SymbolInfo ExternalDefinition::resolve()
{
	if(address)
		return llvm::RuntimeDyld::SymbolInfo((uint64_t)address,
											 llvm::JITSymbolFlags::Exported);
	else
		return llvm::RuntimeDyld::SymbolInfo((uint64_t)lib.SearchForAddressOfSymbol(symbol_name),
											 llvm::JITSymbolFlags::Exported);
}

};
