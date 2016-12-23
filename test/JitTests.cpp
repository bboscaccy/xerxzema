#include <gtest/gtest.h>
#include "../lib/World.h"
#include "../lib/Instruction.h"
#include "../lib/Diagnostics.h"
#include "../lib/JitInvoke.h"
#include "../lib/Session.h"
#include "../lib/Parser.h"
#include <stdio.h>
#include <chrono>
#include <thread>

TEST(TestJit, TestAdd)
{
	xerxzema::World world;
	auto jit = world.jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	p->add_output("bye", world.get_namespace("core")->type("real"));

	p->instruction("add", {"hi", "hi"}, {"bye"});
	jit->compile_namespace(world.get_namespace("core"));

	xerxzema::JitInvoke<double, double> invoker(jit, p);
	ASSERT_EQ(invoker(2), 4.0);
}

TEST(TestJit, TestAddConst)
{
	xerxzema::World world;
	auto jit = world.jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	p->add_output("bye", world.get_namespace("core")->type("real"));

	auto cv = p->constant(42.0);

	p->instruction("add", {"hi", cv.reg->name()}, {"bye"});
	jit->dump_after_codegen();
	jit->compile_namespace(world.get_namespace("core"));

	xerxzema::JitInvoke<double, double> invoker(jit, p);
	ASSERT_EQ(invoker(2), 44.0);

}

TEST(TestJit, TestAddChainConst)
{
	xerxzema::World world;
	auto jit = world.jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	p->add_output("bye", world.get_namespace("core")->type("real"));

	auto temp = p->constant(42.0);
	p->instruction("add", {p->reg_data("hi"), temp}, {p->reg_data("bar")});
	p->instruction("add", {p->reg_data("bar"), temp}, {p->reg_data("bye")});

	jit->compile_namespace(world.get_namespace("core"));
	xerxzema::JitInvoke<double, double> invoker(jit, p);
	ASSERT_EQ(invoker(2), 86.0);

}

TEST(TestJit, TestDelay)
{

	xerxzema::World world;
	auto jit = world.jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	p->add_output("bye", world.get_namespace("core")->type("real"));

	p->instruction("delay", {p->reg_data("hi")}, {p->reg_data("bye")});

	p->instruction("trace", {"hi"}, {});
	jit->dump_after_codegen();
	jit->compile_namespace(world.get_namespace("core"));

	xerxzema::JitInvoke<double, double> invoker(jit, p);
	invoker(0);
	for(int i = 1; i < 20; i++)
	{
		ASSERT_EQ(invoker(i), i-1);
	}
}

TEST(TestJit, TestWhen)
{

	xerxzema::World world;
	auto jit = world.jit();
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

	xerxzema::JitInvoke<double, double, double> invoker(jit, p);
	ASSERT_EQ(invoker(1,2), 3);
	ASSERT_EQ(invoker(2,1), 2);
	ASSERT_EQ(invoker(2,1), 2);
	ASSERT_EQ(invoker(1,2), 3);
}

TEST(TestJit, TestPow)
{
	xerxzema::World world;
	auto jit = world.jit();
	auto p = world.get_namespace("core")->get_program("test");
	p->add_input("hi", world.get_namespace("core")->type("real"));
	p->add_output("bye", world.get_namespace("core")->type("real"));

	auto temp = p->constant(2.0);
	p->instruction("pow", {p->reg_data("hi"), temp}, {p->reg_data("bye")});

	jit->compile_namespace(world.get_namespace("core"));

	xerxzema::JitInvoke<double, double> invoker(jit, p);
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
	auto jit = world.jit();
	jit->compile_namespace(world.get_namespace("core"));
	xerxzema::JitInvoke<double, double, double> invoker(jit, p);
	ASSERT_EQ(invoker(2,3), 5);

	//TODO suppress input variable activations on re-runs when they don't
	//change at all (scheduler based ones)
}

TEST(TestJit, TestSession)
{
	xerxzema::World world;

	world.scheduler()->run_async();
	xerxzema::Session session(&world);
	session.eval("7.0 -> x; 6.0 -> y; 2.0*x*y->z; trace(x); trace(y); trace(z);");
	world.scheduler()->shutdown();
	world.scheduler()->wait();

}

TEST(TestJit, TestProgramCall)
{
	xerxzema::World world;
	auto ns = world.get_namespace("test");
	auto program_str =
R"EOF(
prog foo(x:real) -> y:real
{
	 x + 2.0 -> y;
}
foo(2.0) -> res;
trace(res);
)EOF";
	xerxzema::parse_input(program_str, ns);
	world.jit()->compile_namespace(ns);
	auto p = ns->get_default_program();
	xerxzema::JitInvoke<void> invoker(world.jit(), p);
	invoker();
}
