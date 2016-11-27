#include "Diagnostics.h"

namespace xerxzema
{

void emit_error(const std::string& msg)
{
	std::cout << AnsiRed << AnsiBright;
	std::cout << "Error: ";
	std::cout << AnsiWhite << AnsiBright << msg << '\n';
	std::cout << AnsiReset;
}

void emit_warn(const std::string& msg)
{
	std::cout << AnsiYellow << AnsiBright;
	std::cout << "Warning: ";
	std::cout << AnsiWhite << AnsiBright << msg << '\n';
	std::cout << AnsiReset;
}

void emit_info(const std::string& msg)
{
	std::cout << AnsiMagenta << AnsiBright;
	std::cout << "Info: ";
	std::cout << AnsiWhite << AnsiBright << msg << '\n';
	std::cout << AnsiReset;
}

void emit_debug(const std::string& msg)
{
	std::cout << AnsiBlue << AnsiBright;
	std::cout << "Debug: ";
	std::cout << AnsiWhite << AnsiBright << msg << '\n';
	std::cout << AnsiReset;
}

void emit_error(Token* token, const std::string& msg)
{
	std::cout << AnsiRed << AnsiBright;
	std::cout << "Error: ";
	std::cout << AnsiReset;
	std::cout << '<' << token->line << ',' << token->column << "> ";
	std::cout << AnsiWhite << AnsiBright << msg << '\n';
	if(token->line_data)
	{
		std::cout << AnsiReset;
		std::cout << "    " << *token->line_data << '\n';
		std::cout << "    ";
		std::cout << AnsiDim << AnsiGreen;
		for(size_t i = 1; i < token->column; i++)
		{
			std::cout << '~';
		}
		std::cout << AnsiBright << AnsiRed << "^\n";
	}
	std::cout << AnsiReset;
}

};
