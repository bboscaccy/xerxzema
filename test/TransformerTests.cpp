#include <gtest/gtest.h>
#include "../lib/World.h"
#include "../lib/Instruction.h"
#include "../lib/Transformer.h"
#include "../lib/JitInvoke.h"
#include <stdio.h>

TEST(TestTransformer, TestRegSame)
{
	xerxzema::World world;
	auto ns = world.get_namespace("test");

	auto v0 = ns->get_program("foo.0");
	auto v1 = ns->get_program("foo.1");

	v0->reg("x");
	v0->reg("y");

	v1->reg("x");
	v1->reg("y");

	xerxzema::Transformer trans(v0, v1);
	trans.parse_registers();
	ASSERT_EQ(trans.get_new_registers().size(), 0);
}

TEST(TestTransformer, TestRegDiff)
{
	xerxzema::World world;
	auto ns = world.get_namespace("test");

	auto v0 = ns->get_program("foo.0");
	auto v1 = ns->get_program("foo.1");

	v0->reg("x");

	v1->reg("x");
	v1->reg("y");

	xerxzema::Transformer trans(v0, v1);
	trans.parse_registers();
	ASSERT_EQ(trans.get_new_registers().size(), 1);
}

TEST(TestTransformer, TestRegDiffDelete)
{
	xerxzema::World world;
	auto ns = world.get_namespace("test");

	auto v0 = ns->get_program("foo.0");
	auto v1 = ns->get_program("foo.1");

	v0->reg("x");
	v0->reg("y");

	v1->reg("x");

	xerxzema::Transformer trans(v0, v1);
	trans.parse_registers();
	ASSERT_EQ(trans.get_deleted_registers().size(), 1);
}
