#pragma once

#include <string>
#include <istream>
#include <vector>
#include <memory>

namespace xerxzema
{

enum class TokenType : char
{
	Invalid,
	Eof,
	Number,
	Symbol,
	Operator
};

class Token
{
public:
	Token() = default;
	inline Token(TokenType t, size_t l, size_t c, std::string&& d):
		type(t), line(l), column(c), data(d)
	{
	}

	TokenType type;
	size_t line;
	size_t column;
	std::string data;
};

std::ostream& operator<<(std::ostream& stream, const TokenType& x)
{
	if(x == TokenType::Invalid)
		stream << "invalid";
	else if (x == TokenType::Eof)
		stream << "eof";
	else if (x == TokenType::Number)
		stream << "number";
	else if (x == TokenType::Symbol)
		stream << "symbol";
	else if (x == TokenType::Operator)
		stream << "operator";
	return stream;
}


class Lexer
{
public:
	Lexer(std::istream& input_stream);
	Token* peek();
	std::unique_ptr<Token> get();
	void putback(std::unique_ptr<Token>&& t);

private:
	bool do_number();
	bool do_symbol();
	bool do_operator();
	void read_next_token();
	std::istream& input;
	std::vector<Token> _tokens;
	size_t line;
	size_t col;
	size_t tab_width;
	std::string buffer;
	std::unique_ptr<Token> token;
};



};
