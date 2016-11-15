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
	foo->type(world.get_namespace("core")->type("real"));
	auto nop = std::make_unique<xerxzema::Instruction>();
	nop->input(p->reg("head"));
	nop->output(foo);
	p->instruction(std::move(nop));

	auto bar = p->reg("bar");
	bar->type(world.get_namespace("core")->type("real"));
	auto nop2 = std::make_unique<xerxzema::Instruction>();
	nop2->input(foo);
	nop2->output(bar);
	p->instruction(std::move(nop2));

	auto baz = p->reg("baz");
	baz->type(world.get_namespace("core")->type("real"));
	auto val = std::make_unique<xerxzema::ValueReal>(42.0);
	val->output(baz);
	val->dependent(p->reg("head"));
	p->instruction(std::move(val));

	p->instruction("add", {"hi", "hi"}, {"res"});


	jit->compile_namespace(world.get_namespace("core"));
}
