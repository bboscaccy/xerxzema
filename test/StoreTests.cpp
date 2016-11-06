#include <gtest/gtest.h>
#include "../lib/Store.h"

TEST(TestStore, IsType)
{
	xerxzema::Store store;
	ASSERT_TRUE(store.is_type("unit"));
	ASSERT_TRUE(store.is_type("atom"));
	ASSERT_TRUE(store.is_type("buffer"));
	ASSERT_TRUE(store.is_type("token"));
}

