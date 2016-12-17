#include <iostream>
#include "Jit.h"
#include "World.h"
#include "llvm/Analysis/Passes.h"
#include "RT.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/RuntimeDyld.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"

#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Mangler.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

namespace xerxzema
{

//TODO look into ObjectCache for persistantly stored ir/stuff/whatever
//TODO IndirectStubManager allows you to create stub pointers in IR that
//allow resolving symbols later, e.g fn0 -> trans -> fn2 -> fn3 etc...
//if you just want to manually implement trampolines take a look at
//https://github.com/llvm-mirror/llvm/blob/master/lib/ExecutionEngine/Orc/IndirectionUtils.cpp
//in there make-stub function they create a function that calls a random pointer
//with whatever address you already have in there
//we could do something similar since we need to implement
//transformer logic at each call
//so then calls args become pointer-to-pointers so the transformer can
//fixup input state args? kind-of... but does that get into fights with the
//scheduler callbacks? idk...
//you can add input args to a program during live-coding, if and only if they
//have default values.



static llvm::RuntimeDyld::SymbolInfo get_symbol(void* addr)
{
	return llvm::RuntimeDyld::SymbolInfo((uint64_t)addr, llvm::JITSymbolFlags::Exported);
}

static llvm::RuntimeDyld::SymbolInfo get_symbol(llvm::orc::JITSymbol& symbol)
{
	return llvm::RuntimeDyld::SymbolInfo(symbol.getAddress(), symbol.getFlags());
}


Jit::Jit(World* world) : _world(world), dump_pre_optimization(false), dump_post_optimization(false),
						 target_machine(llvm::EngineBuilder().selectTarget()),
						 data_layout(target_machine->createDataLayout()),
						 compiler(linker, llvm::orc::SimpleCompiler(*target_machine))
{
	scheduler = world->scheduler();
	llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}

std::unique_ptr<llvm::Module> Jit::create_module(Namespace* ns)
{
	auto module = std::make_unique<llvm::Module>(ns->full_name(), _context);
	auto handle = module.get();

	handle->setDataLayout(data_layout);
	handle->setTargetTriple(target_machine->getTargetTriple().getTriple());

	return module;
}

void Jit::compile_namespace(Namespace* ns)
{
	auto module = create_module(ns);
	ns->codegen(module.get(), _context);

	std::vector<std::unique_ptr<llvm::Module>> module_set;
	module_set.push_back(std::move(module));

	compiler.addModuleSet(std::move(module_set),
						  std::make_unique<llvm::SectionMemoryManager>(),
						  std::make_unique<JitResolver>(_world));
	/*
	engines[ns->full_name()]->addGlobalMapping
		(modules[ns->full_name()]->getGlobalVariable("xerxzema_scheduler"),
			 &scheduler);

	if(dump_pre_optimization)
		modules[ns->full_name()]->dump();


	auto fpm = std::make_unique<llvm::legacy::FunctionPassManager>(modules[ns->full_name()]);

	fpm->add(llvm::createPromoteMemoryToRegisterPass());
	fpm->add(llvm::createLoadCombinePass());
	fpm->add(llvm::createInstructionCombiningPass());
	fpm->add(llvm::createDeadCodeEliminationPass());
	fpm->add(llvm::createConstantPropagationPass());
	fpm->add(llvm::createJumpThreadingPass());
	fpm->add(llvm::createReassociatePass());

	fpm->doInitialization();

	for(auto& f: modules[ns->full_name()]->functions())
	{
		fpm->run(f);
	}

	if(dump_post_optimization)
		modules[ns->full_name()]->dump();

	//TODO investigate ways to speed this up ALOT
	//currently it's the dominate factor
	//look into ORC and doing our own sectionManager memoryManager and symbolManager
	engines[ns->full_name()]->finalizeObject();
	*/
}

size_t Jit::get_state_size(Program* program)
{
	auto state_type = program->state_type_value();
	return data_layout.getTypeAllocSize(state_type);
}

void* Jit::get_state_offset(void* target, Program* program, int field)
{
	auto state_type = program->state_type_value();
	auto layout = data_layout.getStructLayout((llvm::StructType*)state_type);
	auto offset = field;
	return ((char*)target)+layout->getElementOffset(offset);
}


void* Jit::get_jitted_function(Program* program)
{
	return (void*)compiler.findSymbol(program->program_name(),false).getAddress();
}

JitResolver::JitResolver(World* world) : world(world)
{

}

llvm::RuntimeDyld::SymbolInfo JitResolver::findSymbol(const std::string &name)
{
	if(name == "xerxzema_scheduler")
		return llvm::RuntimeDyld::SymbolInfo((uint64_t)world->scheduler(),
											 llvm::JITSymbolFlags::Exported);
	return llvm::RuntimeDyld::SymbolInfo(0);
}

llvm::RuntimeDyld::SymbolInfo JitResolver::findSymbolInLogicalDylib(const std::string &name)
{
	//TODO look up functions in world, for right now load them from our process?
	auto addr = llvm::RTDyldMemoryManager::getSymbolAddressInProcess(name);
	return llvm::RuntimeDyld::SymbolInfo(addr, llvm::JITSymbolFlags::Exported);
}

};
