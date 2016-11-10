#include <gtest/gtest.h>
#include "../lib/World.h"
#include <stdio.h>

TEST(TestJit, TestCreation)
{
	xerxzema::World world;
	auto jit = world.create_jit();
}
