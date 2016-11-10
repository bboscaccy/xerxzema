#include "Type.h"

namespace xerxzema
{

Type::Type()
{

}

std::string Atom::name()
{
	return "atom";
}

std::string Unit::name()
{
	return "unit";
}

std::string Real::name()
{
	return "real";
}

std::string Int::name()
{
	return "int";
}

std::string Buffer::name()
{
	return "buffer";
}

std::string Token::name()
{
	return "token";
}

};
