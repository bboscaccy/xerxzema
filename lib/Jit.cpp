#include "Jit.h"
#include "World.h"


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
}

};
