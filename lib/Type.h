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

class TypeProvider
{
public:
	virtual bool is_type(const std::string& name) = 0;
	virtual Type* type(const std::string& name) = 0;
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
