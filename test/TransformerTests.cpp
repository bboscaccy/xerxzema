#include <gtest/gtest.h>
#include "../lib/World.h"
#include "../lib/Instruction.h"
#include "../lib/Transformer.h"
#include "../lib/JitInvoke.h"
#include "../lib/Parser.h"
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

TEST(TestTransformer, TestRegDiffType)
{
	xerxzema::World world;
	auto ns = world.get_namespace("test");

	auto int_type = world.get_namespace("core")->type("int");
	auto real_type = world.get_namespace("core")->type("real");

	auto v0 = ns->get_program("foo.0");
	auto v1 = ns->get_program("foo.1");

	v0->reg("x")->type(int_type);
	v0->reg("y")->type(int_type);

	v1->reg("x")->type(int_type);
	v1->reg("y")->type(real_type);

	xerxzema::Transformer trans(v0, v1);
	trans.parse_registers();
	ASSERT_EQ(trans.get_type_change_registers().size(), 1);
}

TEST(TestTransformer, TestInstSame)
{
	xerxzema::World world;
	auto ns0 = world.get_namespace("test0");
	auto ns1 = world.get_namespace("test1");

	auto p = "12.0 -> x; 13.0 -> y;";

	xerxzema::parse_input(p, ns0);
	xerxzema::parse_input(p, ns1);

	auto v0 = ns0->get_default_program();
	auto v1 = ns1->get_default_program();

	xerxzema::Transformer trans(v0, v1);
	trans.parse_registers();
	trans.parse_instructions();

	ASSERT_EQ(trans.get_new_instructions().size(), 0);
	ASSERT_EQ(trans.get_deleted_instructions().size(), 0);
}

TEST(TestTransformer, TestInstDiff)
{
	xerxzema::World world;
	auto ns0 = world.get_namespace("test0");
	auto ns1 = world.get_namespace("test1");

	auto p0 = "12.0 -> x; 13.0 -> y;";
	auto p1 = "12.0 -> x; 10.0 -> y;";

	xerxzema::parse_input(p0, ns0);
	xerxzema::parse_input(p1, ns1);

	auto v0 = ns0->get_default_program();
	auto v1 = ns1->get_default_program();

	xerxzema::Transformer trans(v0, v1);
	trans.parse_registers();
	trans.parse_instructions();

	ASSERT_EQ(trans.get_new_instructions().size(), 1);
	ASSERT_EQ(trans.get_deleted_instructions().size(), 1);
}


TEST(TestTransformer, TestInstPartialReuse)
{
	xerxzema::World world;
	auto ns0 = world.get_namespace("test0");
	auto ns1 = world.get_namespace("test1");

	auto p0 = "12.0 -> x; 13.0 -> y; x*y->z;";
	auto p1 = "12.0 -> x; 10.0 -> y; x*y->z;";

	xerxzema::parse_input(p0, ns0);
	xerxzema::parse_input(p1, ns1);

	auto v0 = ns0->get_default_program();
	auto v1 = ns1->get_default_program();

	xerxzema::Transformer trans(v0, v1);
	trans.parse_registers();
	trans.parse_instructions();

	ASSERT_EQ(trans.get_new_instructions().size(), 1);
	ASSERT_EQ(trans.get_deleted_instructions().size(), 1);
	ASSERT_EQ(trans.get_reusable_instructions().size(), 2);
}

TEST(TestTransformer, TestInstRetValChange)
{
	xerxzema::World world;
	auto ns0 = world.get_namespace("test0");
	auto ns1 = world.get_namespace("test1");

	auto p0 = "12.0 -> x; 13.0 -> y; x*y->z;";
	auto p1 = "12.0 -> x; 10.0 -> y; x*y->q;";

	xerxzema::parse_input(p0, ns0);
	xerxzema::parse_input(p1, ns1);

	auto v0 = ns0->get_default_program();
	auto v1 = ns1->get_default_program();

	xerxzema::Transformer trans(v0, v1);
	trans.parse_registers();
	trans.parse_instructions();

	ASSERT_EQ(trans.get_new_instructions().size(), 1);
	ASSERT_EQ(trans.get_deleted_instructions().size(), 1);
	ASSERT_EQ(trans.get_reusable_instructions().size(), 2);
	ASSERT_EQ(trans.get_new_registers().size(), 1);
}

//TODO this is exposing a subtle race condition somewhere
TEST(TestTransformer, TestCodeGenSimple)
{
	xerxzema::World world;

	auto ns0 = world.get_namespace("test0");
	auto ns1 = world.get_namespace("test1");

	auto p0 = "12.0 -> x; 13.0 -> y; x*y->z;";
	auto p1 = "12.0 -> x; 10.0 -> y; `x*y->q;";

	xerxzema::parse_input(p0, ns0);
	xerxzema::parse_input(p1, ns1);

	auto v0 = ns0->get_default_program();
	auto v1 = ns1->get_default_program();

	world.jit()->compile_namespace(ns0);
	world.jit()->compile_namespace(ns1);

	xerxzema::Transformer trans(v0, v1);
	trans.parse_registers();
	trans.parse_instructions();

	//auto fn = trans.generate_transformer(world.jit()->context());


}
