#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include "../lib/Lexer.h"

TEST(TestLexer, TestInt)
{
	std::stringstream ss;
	ss << "  12  27\n13\n\t12";
	xerxzema::Lexer lex(ss);
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Int);
	ASSERT_EQ(lex.peek()->column, 3);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Int);
	ASSERT_EQ(lex.peek()->column, 7);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Int);
	ASSERT_EQ(lex.peek()->column, 1);
	ASSERT_EQ(lex.peek()->line, 2);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Int);
	ASSERT_EQ(lex.peek()->column, 5);
	ASSERT_EQ(lex.peek()->line, 3);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Eof);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestLexer, TestOperator)
{
	std::stringstream ss;
	ss << "+ +> +{ +? - -> * ** ! !=";
	xerxzema::Lexer lex(ss);
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Add);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Bang);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::MergeStart);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::When);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Sub);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Result);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Mul);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Pow);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Not);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Ne);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestLexer, TestNumbers)
{
	std::stringstream ss;
	ss << "1 -2 .20 0. 0.2 -1.2 -.2 - .";
	xerxzema::Lexer lex(ss);
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Int);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Int);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Real);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Real);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Real);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Real);;
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Real);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Sub);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Operator);
	lex.get();

}
