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
	Int,
	Real,
	Symbol,
	Add, // +
	Bang, // +>
	MergeStart, // +{
	When, // +?
	Sub, // -
	Result, // ->
	Mul, // *
	Pow, // **
	Not, // !
	Ne, // !=
	Const, // =
	Eq, // ==
	Cond, // ?
	SwitchBegin, // ?{
	With, // :-
	Delimit, // :
	Size, // #
	SeqBegin, // #{
	Gt, // >
	Ge, // >=
	Lt, // <
	Le, // <=
	Div, // /
	Root, // //
	Mod, // /%
	And, // &
	Or, // |
	Xor, // ^
	Sample, // `
	Delay, // ~
	Comment, // ; * \n
	BlockBegin, // {
	BlockEnd, // }
	GroupBegin, // (
	GroupEnd, // )
	BraceBegin, // [
	BraceEnd, // ]
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
	else if (x == TokenType::Int)
		stream << "int";
	else if (x == TokenType::Real)
		stream << "real";
	else if (x == TokenType::Symbol)
		stream << "symbol";
	else if (x == TokenType::Operator)
		stream << "operator";
	else if (x == TokenType::Add)
			stream << "add";
	else if (x == TokenType::Bang)
		stream << "bang";
	else if (x == TokenType::MergeStart)
		stream << "merge-start";
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
	bool do_lexical();
	bool do_comment();
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
