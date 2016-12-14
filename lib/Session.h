#pragma once
#include <string>

namespace xerxzema
{

class World;

class Session
{
public:
	Session(World* world);
	~Session() = default;

	//anon namespace?
	void eval(const std::string& str,
			  const std::string& ns = "ephemeral");
private:
	World* world;

};
};
