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

};
