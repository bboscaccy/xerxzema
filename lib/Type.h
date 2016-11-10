#pragma once
#include <string>

namespace xerxzema
{

class Type
{
public:
	Type();
	virtual std::string name() = 0;
};

class Atom : public Type
{
public:
	std::string name();
};

class Unit : public Type
{
public:
	std::string name();
};

class Real : public Type
{
public:
	std::string name();
};

class Int : public Type
{
public:
	std::string name();
};

class Buffer : public Type
{
public:
	std::string name();
};

class Token : public Type
{
public:
	std::string name();
};

};
