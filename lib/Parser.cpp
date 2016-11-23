#include "Parser.h"

namespace xerxzema
{

AtomNode::AtomNode(std::unique_ptr<Token>&& token) : source_token(std::move(token))
{
}

std::unique_ptr<AtomNode> match_atom(Lexer& lexer)
{
	if(lexer.peek()->type == TokenType::Int ||
	   lexer.peek()->type == TokenType::Real ||
	   lexer.peek()->type == TokenType::Symbol)
	{
		return std::make_unique<AtomNode>(lexer.get());
	}
	else
	{
		return nullptr;
	}
}


};
