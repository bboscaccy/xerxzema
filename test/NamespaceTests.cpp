#include <gtest/gtest.h>
#include "../lib/World.h"
#include "../lib/Instruction.h"
#include "../lib/Jit.h"
#include <stdio.h>

TEST(TestNamespace, TestCreation)
{
	xerxzema::World world;
	world.get_namespace("foo");
	auto bar = world.get_namespace("bar");
	auto baz = bar->get_namespace("baz");
	auto names = world.namespace_list();
	ASSERT_EQ(names.size(), 3);
	ASSERT_EQ(world.get_namespace("foo")->world(), &world);
	ASSERT_EQ(world.namespace_list().size(), 3);
	ASSERT_EQ(bar->namespace_list().size(), 1);
	ASSERT_EQ(bar->full_name(), "bar");
	ASSERT_EQ(baz->full_name(), "bar.baz");
}

TEST(TestNamespace, TestCoreTypes)
{
	xerxzema::World world;
	auto foo = world.get_namespace("foo");
	ASSERT_EQ(true, foo->is_type("unit"));
}

TEST(TestNamespace, TestProgramCreation)
{
	xerxzema::World world;
	auto foo = world.get_namespace("foo");
	auto prog = foo->get_program("test");
	ASSERT_TRUE(foo->is_program("test"));
}


TEST(TestInstructionDef, TestCreation)
{
	xerxzema::World world;
	auto core = world.get_namespace("core");
	auto fake_def = xerxzema::BasicDefinition<xerxzema::Instruction>
		("foobar", {"unit"}, {"real"});
	auto v = fake_def.input_types(core);
	ASSERT_EQ(v[0], core->type("unit"));
	auto v1 = fake_def.output_types(core);
	ASSERT_EQ(v1[0], core->type("real"));

}

TEST(TestNamespace, TestDefaultProgram)
{
	xerxzema::World world;
	auto core = world.get_namespace("core");

	auto prog = core->get_default_program();
	ASSERT_EQ(prog->program_name(), "core.default");

	auto jit = world.jit();
	jit->compile_namespace(core);
	ASSERT_EQ(prog->symbol_name(), "core.core.default");
}
