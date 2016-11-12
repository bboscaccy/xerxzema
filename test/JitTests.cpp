#include <gtest/gtest.h>
#include "../lib/World.h"
#include <stdio.h>

TEST(TestJit, TestCreation)
{
	xerxzema::World world;
	auto jit = world.create_jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	jit->compile_namespace(world.get_namespace("core"));
}
