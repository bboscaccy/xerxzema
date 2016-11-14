#include "Jit.h"
#include "World.h"
#include "llvm/Analysis/Passes.h"


namespace xerxzema
{

Jit::Jit(World* world) : _world(world)
{
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmParser();
	llvm::InitializeNativeTargetAsmPrinter();
	llvm::InitializeNativeTargetDisassembler();
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

	auto fpm = std::make_unique<llvm::legacy::FunctionPassManager>(modules[ns->full_name()]);
	fpm->add(llvm::createPromoteMemoryToRegisterPass());
	fpm->add(llvm::createConstantPropagationPass());
	fpm->add(llvm::createConstantHoistingPass());
	fpm->add(llvm::createDeadCodeEliminationPass());
	fpm->add(llvm::createJumpThreadingPass());
	fpm->add(llvm::createFlattenCFGPass());
	fpm->add(llvm::createGVNHoistPass());

    fpm->doInitialization();

	for(auto& f: *modules[ns->full_name()])
    {
		fpm->run(f);
    }
	modules[ns->full_name()]->dump();
	engines[ns->full_name()]->finalizeObject();
}

};
