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
	current_line = std::make_shared<std::string>();
}

bool Lexer::do_number()
{
	if(!input)
		return false;

	size_t start = col;
	bool starting_dot = false;
	if(input.peek() == '.')
	{
		starting_dot = true;
		input.get();
		col++;
	}

	if(!isdigit(input.peek()))
	{
		if(starting_dot)
		{
			input.putback('.');
			col--;
		}
		return false;
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
		token = std::make_unique<Token>(TokenType::Real, line, start, buffer);
	}
	else
	{
		if(starting_dot)
		{
			token = std::make_unique<Token>(TokenType::Real, line, start, buffer);
		}
		else
		{
			token = std::make_unique<Token>(TokenType::Int, line, start, buffer);
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
	while(isalnum(input.peek()) || input.peek() == '_')
	{
		buffer.push_back(input.get());
		col++;
	}
	if(buffer == "prog")
		token = std::make_unique<Token>(TokenType::ProgKeyword, line, start, buffer);
	else if(buffer == "func")
		token = std::make_unique<Token>(TokenType::FuncKeyword, line, start, buffer);
	else if(buffer == "ugen")
		token = std::make_unique<Token>(TokenType::UgenKeyword, line, start, buffer);
	else
		token = std::make_unique<Token>(TokenType::Symbol, line, start, buffer);
	return true;
}

bool Lexer::do_string()
{
	if(!input)
		return false;

	if(input.peek() != '"')
		return false;

	size_t start = col;
	input.get();
	col++;
	while(input && input.peek() != '"')
	{
		auto c = input.get();
		if(c == '\\')
		{
			col++;
			if(input.peek() == '"')
			{
				buffer.push_back('"');
				col++;
			}
			else if(input.peek() == 'n')
			{
				buffer.push_back('\n');
				col++;
			}
			else if(input.peek() == 't')
			{
				buffer.push_back('\t');
				col++;
			}
			else
			{
				buffer.push_back('\\');
			}
		}
		else
		{
			buffer.push_back(c);
			if(c == '\n')
			{
				col = 0;
				line++;
			}
			else
			{
				col++;
			}
		}
	}
	token = std::make_unique<Token>(TokenType::StringConstant, line, start, buffer);
	return true;

}

bool Lexer::do_lexical()
{
	if(!input)
		return false;

	size_t start = col;
	if(input.peek() == '{')
	{
		input.get();
		col++;
		buffer.push_back('{');
		token = std::make_unique<Token>(TokenType::BlockBegin, line, start, buffer);
		return true;
	}
	else if(input.peek() == '}')
	{
		input.get();
		col++;
		buffer.push_back('}');
		token = std::make_unique<Token>(TokenType::BlockEnd, line, start, buffer);
		return true;
	}
	if(input.peek() == '(')
	{
		input.get();
		col++;
		buffer.push_back('(');
		token = std::make_unique<Token>(TokenType::GroupBegin, line, start, buffer);
		return true;
	}
	else if(input.peek() == ')')
	{
		input.get();
		col++;
		buffer.push_back(')');
		token = std::make_unique<Token>(TokenType::GroupEnd, line, start, buffer);
		return true;
	}
	if(input.peek() == '[')
	{
		input.get();
		col++;
		buffer.push_back('[');
		token = std::make_unique<Token>(TokenType::BraceBegin, line, start, buffer);
		return true;
	}
	else if(input.peek() == ']')
	{
		input.get();
		col++;
		buffer.push_back(']');
		token = std::make_unique<Token>(TokenType::BraceEnd, line, start, buffer);
		return true;
	}

	return false;
}

bool Lexer::do_operator()
{
	if(!input)
		return false;

	if(!is_operator(input.peek()))
	   return false;

	size_t start = col;
	if(input.peek() == '+')
	{
		input.get();
		col++;
		buffer.push_back('+');
		if(input.peek() == '>')
		{
			input.get();
			col++;
			buffer.push_back('>');
			token = std::make_unique<Token>(TokenType::Bang, line, start, buffer);
			return true;
		}
		if(input.peek() == '{')
		{
			input.get();
			col++;
			buffer.push_back('{');
			token = std::make_unique<Token>(TokenType::MergeStart, line, start, buffer);
			return true;
		}
		if(input.peek() == '?')
		{
			input.get();
			col++;
			buffer.push_back('?');
			token = std::make_unique<Token>(TokenType::When, line, start, buffer);
			return true;
		}
		token = std::make_unique<Token>(TokenType::Add, line, start, buffer);
		return true;
	}

	if(input.peek() == '-')
	{
		input.get();
		col++;
		buffer.push_back('-');
		if(input.peek() == '>')
		{
			input.get();
			col++;
			buffer.push_back('>');
			token = std::make_unique<Token>(TokenType::Bind, line, start, buffer);
			return true;
		}
		token = std::make_unique<Token>(TokenType::Sub, line, start, buffer);
		return true;
	}

	if(input.peek() == '*')
	{
		input.get();
		col++;
		buffer.push_back('*');
		if(input.peek() == '*')
		{
			input.get();
			col++;
			buffer.push_back('*');
			token = std::make_unique<Token>(TokenType::Pow, line, start, buffer);
			return true;
		}
		token = std::make_unique<Token>(TokenType::Mul, line, start, buffer);
		return true;
	}

	if(input.peek() == '!')
	{
		input.get();
		col++;
		buffer.push_back('!');
		if(input.peek() == '=')
		{
			input.get();
			col++;
			buffer.push_back('=');
			token = std::make_unique<Token>(TokenType::Ne, line, start, buffer);
			return true;
		}
		token = std::make_unique<Token>(TokenType::Not, line, start, buffer);
		return true;
	}

	if(input.peek() == '=')
	{
		input.get();
		col++;
		buffer.push_back('=');
		if(input.peek() == '=')
		{
			input.get();
			col++;
			buffer.push_back('=');
			token = std::make_unique<Token>(TokenType::Eq, line, start, buffer);
			return true;
		}
		token = std::make_unique<Token>(TokenType::Assign, line, start, buffer);
		return true;
	}

	if(input.peek() == '?')
	{
		input.get();
		col++;
		buffer.push_back('?');
		if(input.peek() == '{')
		{
			input.get();
			col++;
			buffer.push_back('{');
			token = std::make_unique<Token>(TokenType::SwitchBegin, line, start, buffer);
			return true;
		}
		token = std::make_unique<Token>(TokenType::Cond, line, start, buffer);
		return true;
	}

	if(input.peek() == ':')
	{
		input.get();
		col++;
		buffer.push_back(':');
		if(input.peek() == '-')
		{
			input.get();
			col++;
			buffer.push_back('-');
			token = std::make_unique<Token>(TokenType::With, line, start, buffer);
			return true;
		}
		token = std::make_unique<Token>(TokenType::Delimit, line, start, buffer);
		return true;
	}

	if(input.peek() == '#')
	{
		input.get();
		col++;
		buffer.push_back('#');
		if(input.peek() == '{')
		{
			input.get();
			col++;
			buffer.push_back('{');
			token = std::make_unique<Token>(TokenType::SeqBegin, line, start, buffer);
			return true;
		}
		token = std::make_unique<Token>(TokenType::Size, line, start, buffer);
		return true;
	}

	if(input.peek() == '>')
	{
		input.get();
		col++;
		buffer.push_back('>');
		if(input.peek() == '=')
		{
			input.get();
			col++;
			buffer.push_back('=');
			token = std::make_unique<Token>(TokenType::Ge, line, start, buffer);
			return true;
		}
		token = std::make_unique<Token>(TokenType::Gt, line, start, buffer);
		return true;
	}

	if(input.peek() == '<')
	{
		input.get();
		col++;
		buffer.push_back('<');
		if(input.peek() == '=')
		{
			input.get();
			col++;
			buffer.push_back('=');
			token = std::make_unique<Token>(TokenType::Le, line, start, buffer);
			return true;
		}
		token = std::make_unique<Token>(TokenType::Lt, line, start, buffer);
		return true;
	}

	if(input.peek() == '/')
	{
		input.get();
		col++;
		buffer.push_back('/');
		if(input.peek() == '/')
		{
			input.get();
			col++;
			buffer.push_back('/');
			token = std::make_unique<Token>(TokenType::Root, line, start, buffer);
			return true;
		}
		token = std::make_unique<Token>(TokenType::Div, line, start, buffer);
		return true;
	}
	if(input.peek() == '%')
	{
		input.get();
		col++;
		buffer.push_back('%');
		token = std::make_unique<Token>(TokenType::Mod, line, start, buffer);
		return true;
	}
	if(input.peek() == '&')
	{
		input.get();
		col++;
		buffer.push_back('&');
		token = std::make_unique<Token>(TokenType::And, line, start, buffer);
		return true;
	}
	if(input.peek() == '|')
	{
		input.get();
		col++;
		buffer.push_back('|');
		token = std::make_unique<Token>(TokenType::Or, line, start, buffer);
		return true;
	}
	if(input.peek() == '^')
	{
		input.get();
		col++;
		buffer.push_back('^');
		if(input.peek() == '|')
		{
			input.get();
			col++;
			buffer.push_back('|');
			token = std::make_unique<Token>(TokenType::Xor, line, start, buffer);
			return true;
		}
		token = std::make_unique<Token>(TokenType::Pow, line, start, buffer);
		return true;
	}
	if(input.peek() == '`')
	{
		input.get();
		col++;
		buffer.push_back('`');
		token = std::make_unique<Token>(TokenType::Sample, line, start, buffer);
		return true;
	}
	if(input.peek() == '~')
	{
		input.get();
		col++;
		buffer.push_back('~');
		token = std::make_unique<Token>(TokenType::Delay, line, start, buffer);
		return true;
	}
	if(input.peek() == ',')
	{
		input.get();
		col++;
		buffer.push_back(',');
		token = std::make_unique<Token>(TokenType::Seperator, line, start, buffer);
		return true;
	}
	if(input.peek() == '.')
	{
		input.get();
		col++;
		buffer.push_back('.');
		token = std::make_unique<Token>(TokenType::Dot, line, start, buffer);
		return true;
	}
	if(input.peek() == ';')
	{
		input.get();
		col++;
		buffer.push_back(';');
		token = std::make_unique<Token>(TokenType::Term, line, start, buffer);
		return true;
	}

	while(is_operator(input.peek()))
	{
		buffer.push_back(input.get());
		col++;
	}
	token = std::make_unique<Token>(TokenType::Operator, line, start, buffer);
	return true;

}

bool Lexer::do_comment()
{
	if(input.peek() == '/')
	{
		input.get();
		auto start = col;
		col++;
		if(input.peek() == '/')
		{
			col++;
			input.get();
			while(input && input.peek() != '\n')
			{
				buffer.push_back(input.get());
				col++;
			}
			token = std::make_unique<Token>(TokenType::Comment, line, start, buffer);
			return true;
		}
		else
		{
			input.putback('/');
		}
	}
	return false;
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
			*current_line += ' ';
		}
		else if (input.peek() == '\t')
		{
			input.get();
			col += tab_width;
			*current_line += "    ";
		}
		else if(input.peek() == '\n')
		{
			input.get();
			line++;
			col = 1;
			current_line = std::make_shared<std::string>();
		}
	}
	if(do_number())
	{
		*current_line += buffer;
		token->line_data = current_line;
		buffer.clear();
	}
	else if(do_lexical())
	{
		*current_line += buffer;
		token->line_data = current_line;
		buffer.clear();
	}
	else if(do_comment())
	{
		*current_line += buffer;
		token->line_data = current_line;
		buffer.clear();
	}
	else if(do_symbol())
	{
		*current_line += buffer;
		token->line_data = current_line;
		buffer.clear();
	}
	else if(do_operator())
	{
		*current_line += buffer;
		token->line_data = current_line;
		buffer.clear();
	}
	else if(do_string())
	{
		*current_line += buffer;
		token->line_data = current_line;
		buffer.clear();
	}
	else
	{
		if(input)
		{
			buffer.push_back(input.get());
			token = std::make_unique<Token>(TokenType::Invalid, line, col, "");
			*current_line += buffer;
			token->line_data = current_line;

		}
		else
		{
			buffer.push_back(input.get());
			token = std::make_unique<Token>(TokenType::Eof, line, col, "");
			token->line_data = current_line;
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
