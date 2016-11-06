#include "Store.h"

namespace xerxzema
{

Store::Store()
{
	types.emplace("unit", std::make_unique<Unit>());
	types.emplace("atom", std::make_unique<Atom>());
	types.emplace("buffer", std::make_unique<Buffer>());
	types.emplace("token", std::make_unique<Token>());
}

Type* Store::type(const std::string &name)
{
	if(types.find(name) != types.end())
	{
		return types[name].get();
	}
	return nullptr;
}


};
