#include "Type.h"

namespace xerxzema
{

Type::Type()
{

}

std::string Atom::Name()
{
	return "atom";
}

std::string Unit::Name()
{
	return "unit";
}

std::string Buffer::Name()
{
	return "buffer";
}

std::string Token::Name()
{
	return "token";
}

};
