#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include "../lib/Lexer.h"

TEST(TextLexer, TestInt)
{
	std::stringstream ss;
	ss << "  12  27\n13\n\t12";
	xerxzema::Lexer lex(ss);
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Number);
	ASSERT_EQ(lex.peek()->column, 3);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Number);
	ASSERT_EQ(lex.peek()->column, 7);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Number);
	ASSERT_EQ(lex.peek()->column, 1);
	ASSERT_EQ(lex.peek()->line, 2);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Number);
	ASSERT_EQ(lex.peek()->column, 5);
	ASSERT_EQ(lex.peek()->line, 3);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Eof);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TextLexer, TestOperator)
{
	std::stringstream ss;
	ss << "++ -- ^ ~`!@#$%^&*<>,.?\\/=+-";
	xerxzema::Lexer lex(ss);
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Operator);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Operator);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Operator);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Operator);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Eof);
}
