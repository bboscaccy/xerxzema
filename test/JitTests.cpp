#include <gtest/gtest.h>
#include "../lib/World.h"
#include "../lib/Instruction.h"
#include "../lib/Diagnostics.h"
#include "../lib/JitInvoke.h"
#include <stdio.h>
#include <chrono>
#include <thread>

TEST(TestJit, TestAdd)
{
	xerxzema::World world;
	auto jit = world.create_jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	p->add_output("bye", world.get_namespace("core")->type("real"));

	p->instruction("add", {"hi", "hi"}, {"bye"});
	jit->compile_namespace(world.get_namespace("core"));

	xerxzema::JitInvoke<double, double> invoker(jit.get(), p);
	ASSERT_EQ(invoker(2), 4.0);
}

TEST(TestJit, TestAddConst)
{
	xerxzema::World world;
	auto jit = world.create_jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	p->add_output("bye", world.get_namespace("core")->type("real"));

	auto cv = p->constant(42.0);

	p->instruction("add", {"hi", cv.reg->name()}, {"bye"});
	jit->dump_after_codegen();
	jit->compile_namespace(world.get_namespace("core"));

	xerxzema::JitInvoke<double, double> invoker(jit.get(), p);
	ASSERT_EQ(invoker(2), 44.0);

}

TEST(TestJit, TestAddChainConst)
{
	xerxzema::World world;
	auto jit = world.create_jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	p->add_output("bye", world.get_namespace("core")->type("real"));

	auto temp = p->constant(42.0);
	p->instruction("add", {p->reg_data("hi"), temp}, {p->reg_data("bar")});
	p->instruction("add", {p->reg_data("bar"), temp}, {p->reg_data("bye")});

	jit->compile_namespace(world.get_namespace("core"));
	xerxzema::JitInvoke<double, double> invoker(jit.get(), p);
	ASSERT_EQ(invoker(2), 86.0);

}

TEST(TestJit, TestDelay)
{

	xerxzema::World world;
	auto jit = world.create_jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	p->add_output("bye", world.get_namespace("core")->type("real"));

	p->instruction("delay", {p->reg_data("hi")}, {p->reg_data("bye")});

	p->instruction("trace", {"hi"}, {});
	jit->dump_after_codegen();
	jit->compile_namespace(world.get_namespace("core"));

	xerxzema::JitInvoke<double, double> invoker(jit.get(), p);
	invoker(0);
	for(int i = 1; i < 20; i++)
	{
		ASSERT_EQ(invoker(i), i-1);
	}
}

TEST(TestJit, TestWhen)
{

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

	jit->compile_namespace(world.get_namespace("core"));

	xerxzema::JitInvoke<double, double, double> invoker(jit.get(), p);
	ASSERT_EQ(invoker(1,2), 3);
	ASSERT_EQ(invoker(2,1), 2);
	ASSERT_EQ(invoker(2,1), 2);
	ASSERT_EQ(invoker(1,2), 3);
}

TEST(TestJit, TestPow)
{
	xerxzema::World world;
	auto jit = world.create_jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	p->add_output("bye", world.get_namespace("core")->type("real"));

	auto temp = p->constant(2.0);
	p->instruction("pow", {p->reg_data("hi"), temp}, {p->reg_data("bye")});

	jit->compile_namespace(world.get_namespace("core"));

	xerxzema::JitInvoke<double, double> invoker(jit.get(), p);
	ASSERT_EQ(invoker(2), 4);
}

TEST(TestJit, TestSchedulerCallback)
{
	xerxzema::World world;

	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("i0", world.get_namespace("core")->type("real"));
	p->add_input("i1", world.get_namespace("core")->type("real"));
	p->add_output("bye", world.get_namespace("core")->type("real"));

	auto at_time = p->constant_int(20000000);
	p->instruction("schedule_absolute",{at_time}, {p->reg_data("run_it")} );
	p->instruction("add", {p->reg_data("i0"), p->reg_data("i1")},
				   {p->reg_data("bye")}, {p->reg_data("run_it")});
	p->instruction("trace", {p->reg_data("bye")}, {});
	auto jit = world.create_jit();
	jit->dump_after_optimization();
	jit->compile_namespace(world.get_namespace("core"));
	xerxzema::JitInvoke<double, double, double> invoker(jit.get(), p);
	ASSERT_EQ(invoker(2,3), 5);

	xerxzema::JitInvoke<void, double> inv(jit.get(), p);
	inv(2.0);

	xerxzema::JitInvoke<void, void*> inv2(jit.get(), p);
	inv2();
	//TODO suppress input variable activations on re-runs when they don't
	//change at all (scheduler based ones)
}
