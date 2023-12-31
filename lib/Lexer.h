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
	String,
	Add, // +
	Bang, // +>
	MergeStart, // +{
	When, // +?
	Sub, // -
	Bind, // ->
	Mul, // *
	Pow, // ^
	Not, // !
	Ne, // !=
	Assign, // =
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
	Mod, // %
	And, // &
	Or, // |
	Xor, // ^|
	Sample, // `
	Delay, // ~
	Seperator, // ,
	Dot, // .
	Term, // ;
	Comment, // ; * \n
	BlockBegin, // {
	BlockEnd, // }
	GroupBegin, // (
	GroupEnd, // )
	BraceBegin, // [
	BraceEnd, // ]
	Operator,
	ProgKeyword, // prog
	FuncKeyword, // func
	UgenKeyword, // ugen
};

class Token
{
public:
	Token() = default;
	inline Token(TokenType t, size_t l, size_t c, const std::string& d):
		type(t), line(l), column(c), data(d)
	{
	}

	TokenType type;
	size_t line;
	size_t column;
	std::string data;
	std::shared_ptr<std::string> line_data;
};

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
	bool do_string();
	void read_next_token();
	std::istream& input;
	std::vector<Token> _tokens;
	size_t line;
	size_t col;
	size_t tab_width;
	std::string buffer;
	std::unique_ptr<Token> token;
	std::shared_ptr<std::string> current_line;
};



};
