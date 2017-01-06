#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include "../lib/Lexer.h"
#include "../lib/Parser.h"
#include "../lib/Semantic.h"
#include "../lib/World.h"
#include "../lib/Diagnostics.h"

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
	ASSERT_TRUE(sig.is_valid());

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
	ASSERT_TRUE(sig.is_valid());
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
	ASSERT_TRUE(sig.is_valid());
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
	ASSERT_TRUE(sig.is_valid());
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
	ASSERT_TRUE(sig.is_valid());
}

TEST(TestSemantic, TestMalformed)
{
	std::stringstream ss;
	ss << "prog foo(x:real) + y:real, z:real\n" \
		" x + x -> y; ";
	xerxzema::Lexer lexer(ss);

	xerxzema::World world;
	auto ns = world.get_namespace("tests");

	auto expr = xerxzema::expression(lexer);

	xerxzema::HandleCodeDefinitionSignature sig(ns, expr->as_a<xerxzema::CodeDefinition>());
	sig.process();
	ASSERT_FALSE(sig.is_valid());
}

TEST(TestSemantic, TestBadType)
{
	std::stringstream ss;
	ss << "prog foo(x:real) -> y:real, z:Madagascar\n" \
		" x + x -> y; ";
	xerxzema::Lexer lexer(ss);

	xerxzema::World world;
	auto ns = world.get_namespace("tests");

	auto expr = xerxzema::expression(lexer);

	xerxzema::HandleCodeDefinitionSignature sig(ns, expr->as_a<xerxzema::CodeDefinition>());
	sig.process();
	ASSERT_FALSE(sig.is_valid());
}

TEST(TestSemantic, TestBadTop)
{
	std::stringstream ss;
	ss << "prog 2*3 = yeah\n" \
		" x + x -> y; ";
	xerxzema::Lexer lexer(ss);

	xerxzema::World world;
	auto ns = world.get_namespace("tests");

	auto expr = xerxzema::expression(lexer);

	xerxzema::HandleCodeDefinitionSignature sig(ns, expr->as_a<xerxzema::CodeDefinition>());
	sig.process();
	ASSERT_FALSE(sig.is_valid());
}

TEST(TestSemantic, TestHandleStamentSingle)
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

	xerxzema::HandleStatement stmt(sig.program(),
								   expr->as_a<xerxzema::CodeDefinition>()->body.get());

	stmt.process();
	ASSERT_EQ(stmt.count(), 1);
}

TEST(TestSemantic, TestAddExpr)
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

	xerxzema::HandleStatement stmt(sig.program(),
								   expr->as_a<xerxzema::CodeDefinition>()->body.get());

	stmt.process();
	ASSERT_EQ(stmt.count(), 1);
	ASSERT_EQ(sig.program()->instruction_listing().size(), 1);
}

TEST(TestSemantic, TestAddConstExpr)
{
	std::stringstream ss;
	ss << "prog foo(x:real) -> y:real\n" \
		  "    2.0 + x -> y; ";
	xerxzema::Lexer lexer(ss);

	xerxzema::World world;
	auto ns = world.get_namespace("tests");

	auto expr = xerxzema::expression(lexer);

	xerxzema::HandleCodeDefinitionSignature sig(ns, expr->as_a<xerxzema::CodeDefinition>());
	sig.process();

	xerxzema::HandleStatement stmt(sig.program(),
								   expr->as_a<xerxzema::CodeDefinition>()->body.get());

	stmt.process();
	ASSERT_EQ(stmt.count(), 1);
	ASSERT_EQ(sig.program()->instruction_listing().size(), 2);
}

TEST(TestSemantic, TestBadSampleExpr)
{
	std::stringstream ss;
	ss << "prog foo(x:real) -> y:real\n" \
		  "    `2.0 + x -> y; ";
	xerxzema::Lexer lexer(ss);

	xerxzema::World world;
	auto ns = world.get_namespace("tests");

	auto expr = xerxzema::expression(lexer);

	xerxzema::HandleCodeDefinitionSignature sig(ns, expr->as_a<xerxzema::CodeDefinition>());
	sig.process();

	xerxzema::HandleStatement stmt(sig.program(),
								   expr->as_a<xerxzema::CodeDefinition>()->body.get());

	stmt.process();
	ASSERT_EQ(stmt.count(), 1);
	ASSERT_EQ(sig.program()->instruction_listing().size(), 2);
}

TEST(TestSemantic, DefaultHandlerTest)
{
	std::stringstream ss;
	ss << "2.0 + x -> y;" \
		  "3.0 -> x;";
	xerxzema::Lexer lexer(ss);

	xerxzema::World world;
	auto ns = world.get_namespace("tests");

	auto expr = xerxzema::expression(lexer);

	xerxzema::HandleTopLevelExpression sema(ns);
	expr->accept(sema);

	auto p = ns->get_default_program();
	ASSERT_EQ(1, p->instruction_listing().size());
}

TEST(TestSemantic, TestArrayBuilderSyntax)
{
	std::stringstream ss;
	//so essentially
	//[foo] is really just shorthand for array(foo)
	ss << "[2.0, x] -> y;";
	xerxzema::Lexer lexer(ss);

	xerxzema::World world;
	auto ns = world.get_namespace("tests");

	auto expr = xerxzema::expression(lexer);

	xerxzema::HandleTopLevelExpression sema(ns);
	expr->accept(sema);

	auto p = ns->get_default_program();
	ASSERT_EQ(1, p->instruction_listing().size());

}
