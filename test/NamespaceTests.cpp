#include <gtest/gtest.h>
#include "../lib/World.h"

TEST(TestNamespace, TestCreation)
{
	xerxzema::World world;
	world.get_namespace("foo");
	auto bar = world.get_namespace("bar");
	auto baz = bar->get_namespace("baz");
	auto names = world.namespace_list();
	ASSERT_EQ(names.size(), 2);
	ASSERT_EQ(world.get_namespace("foo")->world(), &world);
	ASSERT_EQ(world.namespace_list().size(), 2);
	ASSERT_EQ(bar->namespace_list().size(), 1);
	ASSERT_EQ(bar->full_name(), "bar");
	ASSERT_EQ(baz->full_name(), "bar.baz");
}
