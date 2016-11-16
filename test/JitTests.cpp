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
