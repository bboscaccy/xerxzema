#pragma once

#include <string>
#include <vector>
#include <memory>
#include <istream>

#include "Lexer.h"
#include "Ast.h"

namespace xerxzema
{

std::unique_ptr<Expression> expression(Lexer& lexer, int right_bind = 0);
std::unique_ptr<Expression> null_denotation(Lexer& lexer, std::unique_ptr<Token>&& token);
std::unique_ptr<Expression> left_denotation(Lexer& lexer, std::unique_ptr<Expression>&& expr,
											std::unique_ptr<Token>&& token);
int left_bind(Token* token);

};
