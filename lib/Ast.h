#pragma once

#include <string>
#include <vector>
#include <memory>
#include <istream>

#include "Lexer.h"
#include "VisitorUtils.h"

namespace xerxzema
{

class AstVisitor;

class Expression
{
public:
	Expression(std::unique_ptr<Token>&& token);
	virtual ~Expression() = default;
	virtual std::string show();
	virtual void accept(AstVisitor& v) = 0;

	template<class T>
	bool is_a()
	{
		return util::is_a<AstVisitor, Expression, T>(this);
	}

	template<class T>
	T* as_a()
	{
		return util::as_a<AstVisitor, Expression, T>(this);
	}
	std::unique_ptr<Token> token;
};

class StatementBlock : public Expression
{
public:
	StatementBlock(std::unique_ptr<Token>&& token);
	std::string show();
	void add(std::unique_ptr<Expression>&& expr);
	std::vector<std::unique_ptr<Expression>> expressions;
	void accept(AstVisitor& v);
};

class Statement : public Expression
{
public:
	Statement(std::unique_ptr<Token>&& token, std::unique_ptr<Expression>&& expr);
	std::unique_ptr<Expression> expr;
	std::string show();
	void accept(AstVisitor& v);
};

class WithStatement : public Expression
{
public:
	WithStatement(std::unique_ptr<Token>&& token,
				  std::unique_ptr<Expression>&& with_clause,
				  std::unique_ptr<Expression>&& statements);
	std::unique_ptr<Expression> with_clause;
	std::unique_ptr<Expression> statements;
	std::string show();
	void accept(AstVisitor& v);
};

class CodeDefinition : public Expression
{
public:
	CodeDefinition(std::unique_ptr<Token>&& token,
				   std::unique_ptr<Expression>&& signature,
				   std::unique_ptr<Expression>&& body);
	std::unique_ptr<Expression> signature;
	std::unique_ptr<Expression> body;
	std::string show();
	void accept(AstVisitor& v);
};

class SymbolExpression : public Expression
{
public:
	SymbolExpression(std::unique_ptr<Token>&& token);
	std::string show();
	void accept(AstVisitor& v);
};

class RealExpression : public Expression
{
public:
	RealExpression(std::unique_ptr<Token>&& token);
	std::string show();
	void accept(AstVisitor& v);
};

class StringExpression : public Expression
{
public:
	StringExpression(std::unique_ptr<Token>&& token);
	std::string show();
	void accept(AstVisitor& v);
};

class BinaryExpression : public Expression
{
public:
	virtual ~BinaryExpression() = default;
	BinaryExpression(std::unique_ptr<Token>&& token,
					 std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::unique_ptr<Expression> lhs;
	std::unique_ptr<Expression> rhs;
};

class AnnotationExpression : public BinaryExpression
{
public:
	AnnotationExpression(std::unique_ptr<Token>&& token,
						 std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
	void accept(AstVisitor& v);
};

class AddExpression : public BinaryExpression
{
public:
	AddExpression(std::unique_ptr<Token>&& token,
				  std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
	void accept(AstVisitor& v);
};

class SubExpression : public BinaryExpression
{
public:
	SubExpression(std::unique_ptr<Token>&& token,
				  std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
	void accept(AstVisitor& v);
};

class MulExpression : public BinaryExpression
{
public:
	MulExpression(std::unique_ptr<Token>&& token,
				  std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
	void accept(AstVisitor& v);
};

class DivExpression : public BinaryExpression
{
public:
	DivExpression(std::unique_ptr<Token>&& token,
				  std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
	void accept(AstVisitor& v);
};

class ModExpression : public BinaryExpression
{
public:
	ModExpression(std::unique_ptr<Token>&& token,
				  std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
	void accept(AstVisitor& v);
};

class PowExpression : public BinaryExpression
{
public:
	PowExpression(std::unique_ptr<Token>&& token,
				  std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
	void accept(AstVisitor& v);
};

class LtExpression : public BinaryExpression
{
public:
	LtExpression(std::unique_ptr<Token>&& token,
				  std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
	void accept(AstVisitor& v);
};

class ArgListExpression : public BinaryExpression
{
public:
	ArgListExpression(std::unique_ptr<Token>&& token,
					  std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
	void accept(AstVisitor& v);
};

class GroupExpression : public Expression
{
public:
	GroupExpression(std::unique_ptr<Token>&& token,
					std::unique_ptr<Expression>&& expr);
	std::unique_ptr<Expression> expr;
	std::string show();
	void accept(AstVisitor& v);
};

class NegateExpression : public Expression
{
public:
	NegateExpression(std::unique_ptr<Token>&& token,
					 std::unique_ptr<Expression>&& expr);
	std::unique_ptr<Expression> expr;
	std::string show();
	void accept(AstVisitor& v);
};

class SampleExpression : public Expression
{
public:
	SampleExpression(std::unique_ptr<Token>&& token,
					 std::unique_ptr<Expression>&& expr);
	std::unique_ptr<Expression> expr;
	std::string show();
	void accept(AstVisitor& v);
};

class CallExpression : public Expression
{
public:
	CallExpression(std::unique_ptr<Token>&& token,
				   std::unique_ptr<Expression>&& target, std::unique_ptr<Expression>&& args);
	std::unique_ptr<Expression> target;
	std::unique_ptr<Expression> args;
	std::string show();
	void accept(AstVisitor& v);
};

class AssignExpression : public BinaryExpression
{
public:
	AssignExpression(std::unique_ptr<Token>&& token,
					 std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
	void accept(AstVisitor& v);
};

class BindExpression : public BinaryExpression
{
public:
	BindExpression(std::unique_ptr<Token>&& token,
				   std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
	void accept(AstVisitor& v);
};


class SequenceExpression : public Expression
{
public:
	SequenceExpression(std::unique_ptr<Token>&& token, std::unique_ptr<Expression>&& items);

	std::unique_ptr<Expression> items;
	std::string show();
	void accept(AstVisitor& v);
};

class ListExpression : public Expression
{
public:
	ListExpression(std::unique_ptr<Token>&& token, std::unique_ptr<Expression>&& items);

	std::unique_ptr<Expression> items;
	std::string show();
	void accept(AstVisitor& v);
};

class InvalidNullDetonation : public Expression
{
public:
	InvalidNullDetonation(std::unique_ptr<Token>&& token);
	std::string show();
	void accept(AstVisitor& v);
};

class InvalidLeftDetonation : public Expression
{
public:
	InvalidLeftDetonation(std::unique_ptr<Token>&& token, std::unique_ptr<Expression>&& expr);
	std::unique_ptr<Expression> expr;
	std::string show();
	void accept(AstVisitor& v);
};

class AstVisitor
{
public:
	virtual ~AstVisitor() = default;
	virtual void visit(StatementBlock* e);
	virtual void visit(Statement* e);
	virtual void visit(WithStatement* e);
	virtual void visit(CodeDefinition* e);
	virtual void visit(SymbolExpression* e);
	virtual void visit(RealExpression* e);
	virtual void visit(StringExpression* e);
	virtual void visit(AnnotationExpression* e);
	virtual void visit(AddExpression* e);
	virtual void visit(SubExpression* e);
	virtual void visit(MulExpression* e);
	virtual void visit(DivExpression* e);
	virtual void visit(ModExpression* e);
	virtual void visit(PowExpression* e);
	virtual void visit(LtExpression* e);
	virtual void visit(ArgListExpression* e);
	virtual void visit(GroupExpression* e);
	virtual void visit(SampleExpression* e);
	virtual void visit(CallExpression* e);
	virtual void visit(BindExpression* e);
	virtual void visit(AssignExpression* e);
	virtual void visit(NegateExpression* e);
	virtual void visit(SequenceExpression* e);
	virtual void visit(ListExpression* e);
	virtual void visit(InvalidLeftDetonation* e);
	virtual void visit(InvalidNullDetonation* e);
	virtual void handle_default(Expression* ) = 0;
};
};
