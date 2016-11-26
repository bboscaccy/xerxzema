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

TEST(TestSemantic, TestProgramSingleInput)
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

	auto inputs = sig.program()->input_registers();
	ASSERT_EQ(inputs.size(), 1);
	ASSERT_EQ(inputs[0]->name(), "x");
}

TEST(TestSemantic, TestProgramMultipleInput)
{
	std::stringstream ss;
	ss << "prog foo(x:real, z:real, m:real) -> y:real\n" \
		" x + x -> y; ";
	xerxzema::Lexer lexer(ss);

	xerxzema::World world;
	auto ns = world.get_namespace("tests");

	auto expr = xerxzema::expression(lexer);

	xerxzema::HandleCodeDefinitionSignature sig(ns, expr->as_a<xerxzema::CodeDefinition>());
	sig.process();

	auto inputs = sig.program()->input_registers();
	ASSERT_EQ(inputs.size(), 3);
	ASSERT_EQ(inputs[0]->name(), "x");
	ASSERT_EQ(inputs[1]->name(), "z");
	ASSERT_EQ(inputs[2]->name(), "m");
}

TEST(TestSemantic, TestProgramSingleOutput)
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

	auto outputs = sig.program()->output_registers();
	ASSERT_EQ(outputs.size(), 1);
	ASSERT_EQ(outputs[0]->name(), "y");
}

TEST(TestSemantic, TestProgramMultipleOutput)
{
	std::stringstream ss;
	ss << "prog foo(x:real) -> y:real, z:real\n" \
		" x + x -> y; ";
	xerxzema::Lexer lexer(ss);

	xerxzema::World world;
	auto ns = world.get_namespace("tests");

	auto expr = xerxzema::expression(lexer);

	xerxzema::HandleCodeDefinitionSignature sig(ns, expr->as_a<xerxzema::CodeDefinition>());
	sig.process();

	auto outputs = sig.program()->output_registers();
	ASSERT_EQ(outputs.size(), 2);
	ASSERT_EQ(outputs[0]->name(), "y");
	ASSERT_EQ(outputs[1]->name(), "z");
}
