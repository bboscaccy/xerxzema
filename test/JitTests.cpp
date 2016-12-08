#include <gtest/gtest.h>
#include "../lib/World.h"
#include "../lib/Instruction.h"
#include "../lib/Diagnostics.h"
#include "../lib/JitInvoke.h"
#include <stdio.h>
#include <chrono>
#include <thread>

struct delay_state
{
	bool fired;
	double data;
};

struct test_delay_data
{
	bool resume;
	uint16_t delay_mask;
	delay_state delay_state_data;
};

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
	char state[128] = {0};
	memset(state, 0, 128);
	(*testpointer)(state, &in, &out);
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
	char state[128] = {0};
	memset(state, 0, 128);
	testpointer = (void (*)(void*, double*, double*))jit->get_jitted_function("core", "test");
	double in = 2.0;
	double out = 3.0;
	(*testpointer)(state, &in, &out);
	ASSERT_EQ(out, 44.0);
}

TEST(TestJit, TestAddChainConst)
{
	xerxzema::World world;
	auto jit = world.create_jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	p->add_input("bye", world.get_namespace("core")->type("real"));

	auto temp = p->constant(42.0);
	p->instruction("add", {p->reg_data("hi"), temp}, {p->reg_data("bar")});
	p->instruction("add", {p->reg_data("bar"), temp}, {p->reg_data("bye")});

	jit->compile_namespace(world.get_namespace("core"));

	void (*testpointer)(void*, double*, double*);
	testpointer = (void (*)(void*, double*, double*))jit->get_jitted_function("core", "test");
	double in = 2.0;
	double out = 3.0;
	char state[128] = {0};
	memset(state, 0, 128);
	(*testpointer)(state, &in, &out);
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
	char state[128];
	memset(state, 0, 128);
	(*testpointer)(state, &in, &out);
	ASSERT_EQ(out, 86.0);
}

TEST(TestJit, TestDelay)
{
	/* program TestDelay(hi:real) -> bye:real
	   hi~1 -> bye */

	xerxzema::World world;
	auto jit = world.create_jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	p->add_output("bye", world.get_namespace("core")->type("real"));


	auto dval = std::make_unique<xerxzema::Delay>();
	dval->output(p->reg("bye"));
	dval->input(p->reg("hi"));
	p->instruction(std::move(dval));

	//p->instruction("trace", {"hi"}, {});

	jit->compile_namespace(world.get_namespace("core"));

	void (*testpointer)(void*, double*, double*);
	testpointer = (void (*)(void*, double*, double*))jit->get_jitted_function("core", "test");
	double in = 0.0;
	double out = 0.0;
	test_delay_data state = {0};

	for(int i = 1; i < 1000; i++)
	{
		in = i;
		(*testpointer)(&state, &in, &out);
		ASSERT_EQ(in - 1, out);
	}
}

TEST(TestJit, TestWhen)
{
	/* program TestDelay(hi:real) -> bye:real
	   hi~1 -> bye */

	xerxzema::World world;
	auto jit = world.create_jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("i0", world.get_namespace("core")->type("real"));
	p->add_input("i1", world.get_namespace("core")->type("real"));
	p->add_output("bye", world.get_namespace("core")->type("real"));

	auto x = p->reg("x");
	x->type(world.get_namespace("core")->type("real"));

	p->instruction("lt", {"i0", "i1"}, {"cmp"});
	auto when = std::make_unique<xerxzema::When>();
	when->input(p->reg("cmp"));
	when->input(p->reg("i0"));
	when->output(p->reg("x"));
	p->instruction(std::move(when));

	p->instruction("add", {"x", "i1"}, {"bye"});

	p->instruction("trace", {"bye"}, {});
	p->instruction("trace", {"x"}, {});

	jit->dump_after_codegen();
	jit->compile_namespace(world.get_namespace("core"));

	void (*testpointer)(void*, double*, double*, double*);
	testpointer = (void (*)(void*, double*, double*, double*))jit->get_jitted_function("core", "test");
	double in0 = 1.0;
	double in1 = 2.0;
	double out = 0.0;

	char state[128];
	memset(state, 0, 128);
	(*testpointer)(&state, &in0, &in1, &out);
	ASSERT_EQ(out, 3.0);

	in0 = 2.0;
	in1 = 1.0;
	out = 0.0;
	(*testpointer)(&state, &in0, &in1, &out);
	ASSERT_EQ(out, 2.0);

	in0 = 2.0;
	in1 = 1.0;
	out = 0.0;
	memset(state, 0, 128);
	(*testpointer)(&state, &in0, &in1, &out);
	ASSERT_EQ(out, 1.0);


}

TEST(TestJit, TestPow)
{
	xerxzema::World world;
	auto jit = world.create_jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	p->add_input("bye", world.get_namespace("core")->type("real"));

	auto temp = p->constant(2.0);
	p->instruction("pow", {p->reg_data("hi"), temp}, {p->reg_data("bye")});

	jit->compile_namespace(world.get_namespace("core"));

	void (*testpointer)(void*, double*, double*);
	testpointer = (void (*)(void*, double*, double*))jit->get_jitted_function("core", "test");
	double in = 2.0;
	double out = 3.0;
	char state[128] = {0};
	memset(state, 0, 128);
	(*testpointer)(state, &in, &out);
	ASSERT_EQ(out, 4.0);
}

struct test_state_struct
{
	bool rentry;
	double bar;
	uint16_t s0;
	uint16_t s1;
	uint32_t alpha;
	uint32_t beta;
};

TEST(TestJit, TestStateSize)
{
	xerxzema::World world;
	auto jit = world.create_jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	p->add_input("bye", world.get_namespace("core")->type("real"));

	auto temp = p->constant(2.0);
	p->instruction("pow", {p->reg_data("hi"), temp}, {p->reg_data("bye")});

	jit->compile_namespace(world.get_namespace("core"));

	void (*testpointer)(void*, double*, double*);
	testpointer = (void (*)(void*, double*, double*))jit->get_jitted_function("core", "test");
	double in = 2.0;
	double out = 3.0;
	char state[128] = {0};
	memset(state, 0, 128);
	(*testpointer)(state, &in, &out);
	ASSERT_EQ(out, 4.0);



	ASSERT_EQ(jit->get_state_size("core", "test"), sizeof(test_state_struct));
}

TEST(TestJit, TestJitInvoke)
{
	xerxzema::World world;
	auto jit = world.create_jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	p->add_input("bye", world.get_namespace("core")->type("real"));

	auto temp = p->constant(16.0);
	p->instruction("pow", {p->reg_data("hi"), temp}, {p->reg_data("bye")});

	jit->compile_namespace(world.get_namespace("core"));

	ASSERT_EQ(xerxzema::invoke<double>(jit.get(), "core", "test", 2.0), 256.0);
	ASSERT_EQ(jit->get_state_size("core", "test"), sizeof(test_state_struct));
}
