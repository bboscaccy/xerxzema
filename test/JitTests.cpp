#include <gtest/gtest.h>
#include "../lib/World.h"
#include "../lib/Instruction.h"
#include <stdio.h>

TEST(TestJit, TestAdd)
{
	xerxzema::World world;
	auto jit = world.create_jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	p->add_input("bye", world.get_namespace("core")->type("real"));

	p->instruction("add", {"hi", "hi"}, {"bye"});


	jit->compile_namespace(world.get_namespace("core"));

	void (*testpointer)(void*, double*, double*);
	testpointer = (void (*)(void*, double*, double*))jit->get_jitted_function("core", "test");
	double in = 2.0;
	double out = 3.0;
	(*testpointer)(nullptr, &in, &out);
	ASSERT_EQ(out, 4.0);
}

TEST(TestJit, TestAddConst)
{
	xerxzema::World world;
	auto jit = world.create_jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	p->add_input("bye", world.get_namespace("core")->type("real"));

	auto baz = p->reg("baz");
	baz->type(world.get_namespace("core")->type("real"));
	auto val = std::make_unique<xerxzema::ValueReal>(42.0);
	val->output(baz);
	val->dependent(p->reg("head"));
	p->instruction(std::move(val));


	p->instruction("add", {"hi", "baz"}, {"bye"});


	jit->compile_namespace(world.get_namespace("core"));

	void (*testpointer)(void*, double*, double*);
	testpointer = (void (*)(void*, double*, double*))jit->get_jitted_function("core", "test");
	double in = 2.0;
	double out = 3.0;
	(*testpointer)(nullptr, &in, &out);
	ASSERT_EQ(out, 44.0);
}

TEST(TestJit, TestAddChainConst)
{
	xerxzema::World world;
	auto jit = world.create_jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	p->add_input("bye", world.get_namespace("core")->type("real"));

	auto baz = p->reg("baz");
	baz->type(world.get_namespace("core")->type("real"));
	auto val = std::make_unique<xerxzema::ValueReal>(42.0);
	val->output(baz);
	val->dependent(p->reg("head"));
	p->instruction(std::move(val));


	p->instruction("add", {"hi", "baz"}, {"bar"});
	p->instruction("add", {"bar", "baz"}, {"bye"});


	jit->compile_namespace(world.get_namespace("core"));

	void (*testpointer)(void*, double*, double*);
	testpointer = (void (*)(void*, double*, double*))jit->get_jitted_function("core", "test");
	double in = 2.0;
	double out = 3.0;
	(*testpointer)(nullptr, &in, &out);
	ASSERT_EQ(out, 86.0);
}

TEST(TestJit, TestAddChainConstDelay)
{
	xerxzema::World world;
	auto jit = world.create_jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	p->add_input("bye", world.get_namespace("core")->type("real"));

	auto baz = p->reg("baz");
	baz->type(world.get_namespace("core")->type("real"));
	auto val = std::make_unique<xerxzema::ValueReal>(42.0);
	val->output(baz);
	val->dependent(p->reg("head"));
	p->instruction(std::move(val));

	auto dbaz = p->reg("dbaz");
	dbaz->type(world.get_namespace("core")->type("real"));
	auto dval = std::make_unique<xerxzema::Delay>();
	dval->output(dbaz);
	dval->input(baz);

	p->instruction(std::move(dval));


	p->instruction("add", {"hi", "baz"}, {"bar"});
	p->instruction("add", {"bar", "baz"}, {"bye"});


	jit->compile_namespace(world.get_namespace("core"));

	void (*testpointer)(void*, double*, double*);
	testpointer = (void (*)(void*, double*, double*))jit->get_jitted_function("core", "test");
	double in = 2.0;
	double out = 3.0;
	(*testpointer)(nullptr, &in, &out);
	ASSERT_EQ(out, 86.0);
}
