#include "Semantic.h"

namespace xerxzema
{

HandleTopLevelExpression::HandleTopLevelExpression(Namespace* n) : ns(n)
{
}

void HandleTopLevelExpression::handle_default(xerxzema::Expression *e)
{
	//TODO flag this as invalid
	valid = false;
}

void HandleTopLevelExpression::visit(xerxzema::CodeDefinition *e)
{
	valid = true;
}

};
