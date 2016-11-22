#include "Lexer.h"
#include <ctype.h>

namespace xerxzema
{

inline bool is_operator(int c)
{
	return c == '~' || c == '`' || c == '!' || c == '@' || c == '#' || c == '$'
		|| c == '%' || c == '^' || c == '&' || c == '*' || c == '-' || c == '+'
		|| c == '=' || c == ',' || c == '.' || c == '?' || c == '/' || c == '\\'
		|| c == '<' || c == '>' || c == '|' || c == ':' || c == ';';

}

inline bool is_whitespace(int c)
{
	return c == ' ' || c == '\n' || c == '\t';
}

Lexer::Lexer(std::istream& input_stream) : input(input_stream), line(1), col(1),
										   tab_width(4)

{
}

bool Lexer::do_number()
{
	if(!input)
		return false;

	size_t start = col;
	bool is_negative = false;
	bool starting_dot = false;
	if(input.peek() == '-')
	{
		is_negative = true;
		input.get();
		col++;
	}

	if(input.peek() == '.')
	{
		starting_dot = true;
		input.get();
		col++;
	}

	if(!isdigit(input.peek()))
	{
		if(is_negative)
		{
			input.putback('-');
			col--;
		}
		if(starting_dot)
		{
			input.putback('.');
			col--;
		}
		return false;
	}

	if(is_negative)
	{
		buffer.push_back('-');
	}
	if(starting_dot)
	{
		buffer.push_back('0');
		buffer.push_back('.');
	}

	while(isdigit(input.peek()))
	{
		buffer.push_back(input.get());
		col++;
	}
	if(!starting_dot && input.peek() == '.')
	{
		buffer.push_back(input.get());
		col++;
		while(isdigit(input.peek()))
		{
			buffer.push_back(input.get());
			col++;
		}
		token = std::make_unique<Token>(TokenType::Real, line, start, std::move(buffer));
	}
	else
	{
		if(starting_dot)
		{
			token = std::make_unique<Token>(TokenType::Real, line, start, std::move(buffer));
		}
		else
		{
			token = std::make_unique<Token>(TokenType::Int, line, start, std::move(buffer));
		}
	}
	return true;
}

bool Lexer::do_symbol()
{
	if(!input)
		return false;

	if(input.peek() != '_' && !isalpha(input.peek()))
	   return false;

	size_t start = col;
	while(isalnum(input.peek() || input.peek() == '_'))
	{
		buffer.push_back(input.get());
		col++;
	}
	token = std::make_unique<Token>(TokenType::Symbol, line, start, std::move(buffer));
	return true;
}

bool Lexer::do_operator()
{
	if(!input)
		return false;

	if(!is_operator(input.peek()))
	   return false;

	size_t start = col;
	while(is_operator(input.peek()))
	{
		buffer.push_back(input.get());
		col++;
	}
	token = std::make_unique<Token>(TokenType::Operator, line, start, std::move(buffer));
	return true;

}

void Lexer::read_next_token()
{
	if(!input)
		token = std::make_unique<Token>(TokenType::Eof, line, col, "");
	while(is_whitespace(input.peek()))
	{
		if(input.peek() == ' ')
		{
			input.get();
			col++;
		}
		else if (input.peek() == '\t')
		{
			input.get();
			col += tab_width;
		}
		else if(input.peek() == '\n')
		{
			input.get();
			line++;
			col = 1;
		}
	}
	if(do_number())
	{
	}
	else if(do_operator())
	{
	}
	else if(do_symbol())
	{
	}
	else
	{
		if(input)
		{
			buffer.push_back(input.get());
			token = std::make_unique<Token>(TokenType::Invalid, line, col, "");
		}
		else
		{
			buffer.push_back(input.get());
			token = std::make_unique<Token>(TokenType::Eof, line, col, "");
		}
	}
}
Token* Lexer::peek()
{
	if(!token)
		read_next_token();
	return token.get();
}

std::unique_ptr<Token> Lexer::get()
{
	if(!token)
		read_next_token();
	return std::move(token);
}

void Lexer::putback(std::unique_ptr<Token>&& t)
{
	token = std::move(t);
}
};
