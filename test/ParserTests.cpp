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

TEST(TestParser, TestCall)
{
	std::stringstream ss;
	ss << "f(x)";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(call (symbol f) (symbol x))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestCallChain)
{
	std::stringstream ss;
	ss << "f(x,y,z)";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(),
			  "(call (symbol f) (arg-list (arg-list (symbol x) (symbol y)) (symbol z)))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestSubExpr)
{

	std::stringstream ss;
	ss << "a - b";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(sub (symbol a) (symbol b))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestNegExpr)
{

	std::stringstream ss;
	ss << "a + -b + c";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(add (add (symbol a) (negate (symbol b))) (symbol c))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestAssignExpr)
{

	std::stringstream ss;
	ss << "a = -b + c";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(assign (symbol a) (add (negate (symbol b)) (symbol c)))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestBindExpr)
{

	std::stringstream ss;
	ss << "f(x) -> y,z";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(),
			  "(bind (call (symbol f) (symbol x)) (arg-list (symbol y) (symbol z)))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestBindAssignExpr)
{

	std::stringstream ss;
	ss << "y = f(x) -> y,z;";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(),
"[(bind (assign (symbol y) (call (symbol f) (symbol x))) (arg-list (symbol y) (symbol z)))]");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestBadOperator)
{

	std::stringstream ss;
	ss << "+ x";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(invalid-null (token '+'))");
}

TEST(TestParser, TestDivExpr)
{

	std::stringstream ss;
	ss << "a / b / c";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(div (div (symbol a) (symbol b)) (symbol c))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestModExpr)
{

	std::stringstream ss;
	ss << "a+b%c";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(add (symbol a) (mod (symbol b) (symbol c)))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestPowExpr)
{

	std::stringstream ss;
	ss << "a*b^c^d";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(mul (symbol a) (pow (symbol b) (pow (symbol c) (symbol d))))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestStatementBlock)
{

	std::stringstream ss;
	ss << "{ x->y;y*y->z; }";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(),
		  "{[(bind (symbol x) (symbol y))][(bind (mul (symbol y) (symbol y)) (symbol z))]}");
}

TEST(TestParser, TestWith)
{

	std::stringstream ss;
	ss << ":- a,b f(x);";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(),
			  "[with (arg-list (symbol a) (symbol b)) [(call (symbol f) (symbol x))]]");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestSampleExpr)
{

	std::stringstream ss;
	ss << "`a + b -> c";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(bind (add (sample (symbol a)) (symbol b)) (symbol c))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestAnnotation)
{

	std::stringstream ss;
	ss << "(int:x, int:y)";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(),
 "(group (arg-list (annotation (symbol int) (symbol x)) (annotation (symbol int) (symbol y))))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestBasicDecl)
{

	std::stringstream ss;
	ss << "(int:x, int:y) -> int:z";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(),
			  "(bind (group (arg-list (annotation (symbol int) (symbol x))" \
" (annotation (symbol int) (symbol y)))) (annotation (symbol int) (symbol z)))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestSimpleProg)
{
	std::stringstream ss;
	ss << "prog foo(int:x, int:y) -> int:z\n"	\
		  "  x + y -> z;";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(),
			  "[prog (bind (call (symbol foo) (arg-list (annotation (symbol int) (symbol x)) "\
			  "(annotation (symbol int) (symbol y)))) (annotation (symbol int) (symbol z))) "\
			  "[(bind (add (symbol x) (symbol y)) (symbol z))]]");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
	ASSERT_TRUE(expr->is_a<xerxzema::CodeDefinition>());
}

TEST(TestParser, TestMissingParen)
{

	std::stringstream ss;
	ss << "(x + a) * (y + b";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestRealExpr)
{

	std::stringstream ss;
	ss << "12.0 + b -> c";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(bind (add (real 12.0) (symbol b)) (symbol c))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestSeqExpr)
{

	std::stringstream ss;
	ss << "#{ 1.0, 2.0, 3.0 }";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(seq (arg-list (arg-list (real 1.0) (real 2.0)) (real 3.0)))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestListExpr)
{

	std::stringstream ss;
	ss << "[ 1.0, 2.0, 3.0 ]";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(list (arg-list (arg-list (real 1.0) (real 2.0)) (real 3.0)))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestStringExpr)
{

	std::stringstream ss;
	ss << "\"hello world!\"";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(string hello world!)");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestWhenExpr)
{

	std::stringstream ss;
	ss << " +? x, y -> z";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(bind (when (arg-list (symbol x) (symbol y))) (symbol z))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestBangExpr)
{

	std::stringstream ss;
	ss << " +> x";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(bang (symbol x))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestMergeExpr)
{

	std::stringstream ss;
	ss << "+{ x, y ,z }";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(merge (arg-list (arg-list (symbol x) (symbol y)) (symbol z)))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}

TEST(TestParser, TestCondExpr)
{

	std::stringstream ss;
	ss << " ? x, y -> z";
	xerxzema::Lexer lexer(ss);

	auto expr = xerxzema::expression(lexer);
	ASSERT_EQ(expr->show(), "(bind (cond (arg-list (symbol x) (symbol y))) (symbol z))");
	ASSERT_EQ(lexer.peek()->type, xerxzema::TokenType::Eof);
}
