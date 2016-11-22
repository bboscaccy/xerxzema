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
	ss << "+ +> +{ +? - -> * ** ! != = == ? ?{ : :- # #{ > >= < <= / // /% & | ^ ` ~";
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
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Const);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Eq);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Cond);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::SwitchBegin);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Delimit);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::With);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Size);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::SeqBegin);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Gt);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Ge);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Lt);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Le);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Div);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Root);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Mod);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::And);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Or);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Xor);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Sample);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Delay);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestLexer, TestLexicalScope)
{
	std::stringstream ss;
	ss << "(){}[]";
	xerxzema::Lexer lex(ss);
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::GroupBegin);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::GroupEnd);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::BlockBegin);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::BlockEnd);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::BraceBegin);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::BraceEnd);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestLexer, TestComments)
{
	std::stringstream ss;
	ss << "( ;this is a bunch of garbage that is ignored...\n";
	ss << "12";
	xerxzema::Lexer lex(ss);
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::GroupBegin);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Comment);
	lex.get();
	ASSERT_EQ(lex.peek()->type, xerxzema::TokenType::Int);
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
