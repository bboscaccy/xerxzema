#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include "../lib/Lexer.h"
#include "../lib/Parser.h"
#include "../lib/Semantic.h"
#include "../lib/World.h"
#include "../lib/Diagnostics.h"

TEST(TestScheduler, TestSingleInitSingle)
{
	xerxzema::World world;
	world.scheduler()->exit_when_empty();
	world.scheduler()->run();
}

TEST(TestScheduler, TestInitMulti)
{
	xerxzema::World world;
	world.scheduler()->exit_when_empty();
	world.scheduler()->run_async();
	world.scheduler()->wait();
}

TEST(TestScheduler, TestInitMultiSignalLater)
{
	xerxzema::World world;
	world.scheduler()->run_async();
	world.scheduler()->shutdown();
	world.scheduler()->wait();
}
