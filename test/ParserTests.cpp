#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include "../lib/Lexer.h"
#include "../lib/Parser.h"

TEST(TestParser, TestAddExpr)
{

	std::stringstream ss;
	ss << "a + b";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(add (symbol a) (symbol b))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestMulExpr)
{

	std::stringstream ss;
	ss << "a * b";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(mul (symbol a) (symbol b))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestAddMulExpr)
{

	std::stringstream ss;
	ss << "a + b * c";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(add (symbol a) (mul (symbol b) (symbol c)))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestMulAddExpr)
{
	std::stringstream ss;
	ss << "a * b + c";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(add (mul (symbol a) (symbol b)) (symbol c))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestArgListExpr)
{
	std::stringstream ss;
	ss << "a, b + b0, c";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(),
			  "(arg-list (arg-list (symbol a) (add (symbol b) (symbol b0))) (symbol c))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestGroupExpr)
{
	std::stringstream ss;
	ss << "(a * b)";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(group (mul (symbol a) (symbol b)))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestGroupExprPrec)
{
	std::stringstream ss;
	ss << "(a + b) * c";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(mul (group (add (symbol a) (symbol b))) (symbol c))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestCallExprCompound)
{
	std::stringstream ss;
	ss << "f, x(a, d)";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(),
			  "(arg-list (symbol f) (call (symbol x) (arg-list (symbol a) (symbol d))))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestGroupExprPrecLeft)
{
	std::stringstream ss;
	ss << "a + (b * c)";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(add (symbol a) (group (mul (symbol b) (symbol c))))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}



TEST(TestParser, TestCallExprBin)
{
	std::stringstream ss;
	ss << "f + x(a)";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(),
			  "(add (symbol f) (call (symbol x) (symbol a)))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}
