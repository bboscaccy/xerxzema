#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include "../lib/Lexer.h"
#include "../lib/Parser.h"
#include "../lib/Semantic.h"
#include "../lib/World.h"

TEST(TestSemantic, TestValidateTop)
{
	std::stringstream ss;
	ss << "prog foo(x:real) -> y:real\n" \
		" x + x -> y; ";
	xerxzema::Lexer lexer(ss);

	xerxzema::World world;
	auto ns = world.get_namespace("tests");

	xerxzema::HandleTopLevelExpression top_level(ns);
	auto expr = xerxzema::expression(lexer);

	expr->accept(top_level);
	ASSERT_TRUE(top_level.is_valid());
}

TEST(TestSemantic, TestFindProgamName)
{
	std::stringstream ss;
	ss << "prog foo(x:real) -> y:real\n" \
		" x + x -> y; ";
	xerxzema::Lexer lexer(ss);

	xerxzema::World world;
	auto ns = world.get_namespace("tests");

	auto expr = xerxzema::expression(lexer);
	xerxzema::HandleCodeDefinitionSignature sig(ns, expr->as_a<xerxzema::CodeDefinition>());
	sig.process();

	ASSERT_EQ(sig.program()->program_name(), "foo");

}
