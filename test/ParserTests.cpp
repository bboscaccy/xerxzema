#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include "../lib/Lexer.h"
#include "../lib/Parser.h"

TEST(TestParser, TestAtom)
{
	std::stringstream ss;
	ss << "aa 3 12 2.0";
	xerxzema::Lexer lex(ss);

	auto n = xerxzema::match_atom(lex);
	ASSERT_NE(n.get(), nullptr);

	auto n1 = xerxzema::match_atom(lex);
	ASSERT_NE(n1.get(), nullptr);

	auto n2 = xerxzema::match_atom(lex);
	ASSERT_NE(n2.get(), nullptr);

	auto n3 = xerxzema::match_atom(lex);
	ASSERT_NE(n3.get(), nullptr);

	auto n4 = xerxzema::match_atom(lex);
	ASSERT_EQ(n4.get(), nullptr);
}
