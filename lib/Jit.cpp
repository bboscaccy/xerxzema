#include "Jit.h"
#include "World.h"
#include "llvm/Analysis/Passes.h"
#include "RT.h"

namespace xerxzema
{

Jit::Jit(World* world) : _world(world), dump_pre_optimization(false), dump_post_optimization(false)
{
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmParser();
	llvm::InitializeNativeTargetAsmPrinter();
	llvm::InitializeNativeTargetDisassembler();
	scheduler = world->scheduler();
}

void Jit::create_module(Namespace* ns)
{
	auto module = std::make_unique<llvm::Module>(ns->full_name(), _context);
	auto handle = module.get();
	std::string err_str;
	auto engine = std::unique_ptr<llvm::ExecutionEngine>(llvm::EngineBuilder(std::move(module))
														 .setEngineKind(llvm::EngineKind::JIT)
														 .setErrorStr(&err_str).create());
	handle->setDataLayout(engine->getTargetMachine()->createDataLayout());
	handle->setTargetTriple(engine->getTargetMachine()->getTargetTriple().getTriple());

	modules[ns->full_name()] = handle;
	engines[ns->full_name()] = std::move(engine);
}

void Jit::compile_namespace(Namespace* ns)
{
	create_module(ns);
	ns->codegen(modules[ns->full_name()], _context);
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


	engines[ns->full_name()]->finalizeObject();

}

size_t Jit::get_state_size(const std::string& ns, const std::string &name)
{
	auto fn = modules[ns]->getFunction(name);
	auto state_ptr_type = (*fn->getArgumentList().begin()).getType();
	auto state_type = state_ptr_type->getPointerElementType();
	return modules[ns]->getDataLayout().getTypeAllocSize(state_type);
}

void* Jit::get_jitted_function(const std::string& ns, const std::string &name)
{
	auto fn = modules[ns]->getFunction(name);
	auto state_type = (*fn->getArgumentList().begin()).getType();
	modules[ns]->getDataLayout().getTypeAllocSize(state_type);
	return engines[ns]->getPointerToFunction(fn);
}

};
