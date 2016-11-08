#pragma once
#include <string>

namespace xerxzema
{

class Type
{
public:
	Type();
	virtual std::string Name() = 0;
};

class Atom : public Type
{
public:
	std::string Name();
};

class Unit : public Type
{
public:
	std::string Name();
};

class Buffer : public Type
{
public:
	std::string Name();
};

class Token : public Type
{
public:
	std::string Name();
};

};
