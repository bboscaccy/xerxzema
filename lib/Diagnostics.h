#pragma once

#include <iostream>
#include <string>

#include "Lexer.h"

namespace xerxzema
{

constexpr auto AnsiRed = "\x1B[31m";
constexpr auto AnsiGreen = "\x1B[32m";
constexpr auto AnsiYellow = "\x1B[33m";
constexpr auto AnsiBlue = "\x1B[34m";
constexpr auto AnsiMagenta = "\x1B[35m";
constexpr auto AnsiCyan = "\x1B[36m";
constexpr auto AnsiWhite = "\x1B[37m";
constexpr auto AnsiReset = "\x1B[0m";
constexpr auto AnsiBright = "\x1B[1m";
constexpr auto AnsiDim = "\x1B[2m";
constexpr auto AnsiUnderline = "\x1B[3m";
constexpr auto AnsiBlink = "\x1B[4m";
constexpr auto AnsiReverse = "\x1B[7m";
constexpr auto AnsiHidden = "\x1B[8m";

void emit_error(const std::string& msg);
void emit_warn(const std::string& msg);
void emit_info(const std::string& msg);
void emit_debug(const std::string& msg);

void emit_error(Token* token, const std::string& msg);

};
