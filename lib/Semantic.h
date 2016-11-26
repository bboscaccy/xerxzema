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
};
