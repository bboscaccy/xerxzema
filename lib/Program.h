#include <map>
#include <string>
#include <memory>
#include "Register.h"

namespace xerxzema
{

class Program
{
public:
	Program(const std::string& name);
	void add_input(const std::string& name, Type* type);
	void add_output(const std::string& name, Type* type);

private:
	std::map<std::string, std::unique_ptr<Register>> registers;
	std::string _name;
};


};
