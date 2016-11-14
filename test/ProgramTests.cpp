#include <gtest/gtest.h>
#include "../lib/World.h"
#include "../lib/Instruction.h"
#include <stdio.h>

TEST(TestProgramBuild, TestVarInfer)
{
	xerxzema::World world;
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	p->instruction("nop", {"foo"}, {"bar"});
	p->instruction("nop", {"head"}, {"foo"});
	ASSERT_NE(p->reg("foo")->type(), nullptr);
	if(p->reg("foo")->type())
		ASSERT_EQ(p->reg("foo")->type()->name(), "unit");
	ASSERT_NE(p->reg("bar")->type(), nullptr);
	if(p->reg("bar")->type())
		ASSERT_EQ(p->reg("bar")->type()->name(), "unit");
}
