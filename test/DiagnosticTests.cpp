#include <gtest/gtest.h>
#include "../lib/Diagnostics.h"

TEST(TestDiagnostics, TestBasic)
{
	xerxzema::emit_error("this is an error!");
	xerxzema::emit_warn("warning! warning!");
	xerxzema::emit_info("fyi...");
	xerxzema::emit_debug("i'm sorry?");
}


TEST(TestDiagnostics, TestToken)
{
	xerxzema::Token token;
	token.line = 1;
	token.column = 1;
	xerxzema::emit_error(&token, "this is an error!");
}
