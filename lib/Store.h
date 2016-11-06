#include "Type.h"
#include <memory>
#include <string>
#include <map>

namespace xerxzema
{

class Store
{
public:
	Store();
	inline bool is_type(const std::string& name)
	{
		return type(name) != nullptr;
	}

	Type* type(const std::string& name);

private:
	std::map<std::string, std::unique_ptr<Type>> types;
};

};
