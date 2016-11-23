#pragma once

#include <string>
#include <vector>
#include <memory>
#include <istream>

#include "Lexer.h"

namespace xerxzema
{

class AtomNode
{
public:
	AtomNode(std::unique_ptr<Token>&& source);
	inline Token* token() { return source_token.get(); }
private:
	std::unique_ptr<Token> source_token;
};

std::unique_ptr<AtomNode> match_atom(Lexer& lexer);


};
