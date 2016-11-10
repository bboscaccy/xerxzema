#include <gtest/gtest.h>
#include "../lib/World.h"
#include <stdio.h>

TEST(TestJit, TestCreation)
{
	xerxzema::World world;
	auto jit = world.create_jit();
	world.get_namespace("core")->get_program("test");
	jit->compile_namespace(world.get_namespace("core"));
}
