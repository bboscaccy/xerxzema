#pragma once

#include "Ast.h"
#include "Namespace.h"
#include "Program.h"

namespace xerxzema
{

class HandleTopLevelExpression : public AstVisitor
{
public:
	HandleTopLevelExpression(Namespace* ns);
	void visit(CodeDefinition* e);
	void handle_default(Expression* e);
	inline bool is_valid() { return valid; }
private:
	Namespace* ns;
	bool valid;
};


class HandleCodeDefinitionSignature : public AstVisitor
{
	enum class ProcessState
	{
		Entry,
		Lhs,
		Name,
		InputArgs,
		GetArgName,
		GetArgType,
		OutputArgs
	};

public:
	HandleCodeDefinitionSignature(Namespace* ns, CodeDefinition* def);
	void process();
	void visit(BindExpression* e);
	void visit(CallExpression* e);
	void visit(SymbolExpression* e);
	void visit(ArgListExpression* e);
	void visit(AnnotationExpression* e);
	void handle_default(Expression* e);

	inline Program* program() { return prog; }
	inline bool is_valid() { return valid; }

private:
	Namespace* ns;
	bool valid;
	CodeDefinition* def;
	ProcessState state;
	std::string name;
	Program* prog;
	std::string current_arg_name;
	std::string current_arg_type;
};

class HandleStatement : public AstVisitor
{
public:
	HandleStatement(Program* program, Expression* expr);
	void process();
	void visit(Statement* e);
	void visit(StatementBlock* e);
	void visit(WithStatement* e);

	void handle_default(Expression* e);

	inline bool is_valid() { return valid; }
	inline bool count() { return counter; }

private:
	Program* program;
	Expression* expr;
	bool valid;
	size_t counter;
	std::vector<RegisterData> extra_dependencies;
};

class HandleExpression : public AstVisitor
{
public:
	HandleExpression(Program* program, Expression* expr,
					 const std::vector<RegisterData>& result = {},
					 const std::vector<RegisterData>& dependencies = {});
	void process();
	void visit(SymbolExpression* e);
	void visit(SampleExpression* e);
	void visit(RealExpression* e);
	void visit(ArgListExpression* e);
	void visit(AddExpression* e);
	void visit(SubExpression* e);
	void visit(MulExpression* e);
	void visit(DivExpression* e);
	void visit(GroupExpression* e);
	void visit(BindExpression* e);
	void visit(CallExpression* e);
	void handle_default(Expression* e);

	inline std::vector<RegisterData> results() { return result; }
private:
	void do_binary_instruction(Expression* parent, Expression* lhs,
							   Expression* rhs, const std::string& op);
	Program* program;
	Expression* expr;
	bool valid;
	std::vector<RegisterData> result;
	std::vector<RegisterData> dependencies;
};

};
