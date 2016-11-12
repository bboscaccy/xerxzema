#include <gtest/gtest.h>
#include "../lib/World.h"
#include "../lib/Instruction.h"
#include <stdio.h>

TEST(TestJit, TestCreation)
{
	xerxzema::World world;
	auto jit = world.create_jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	auto foo = p->reg("foo");
	auto nop = std::make_unique<xerxzema::Instruction>();
	nop->input(p->reg("head"));
	nop->output(foo);
	p->instruction(std::move(nop));
	jit->compile_namespace(world.get_namespace("core"));
}
