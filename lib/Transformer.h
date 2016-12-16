#pragma once
#include <vector>
#include <string>

#include "Program.h"

namespace xerxzema
{

class Transfomer
{
public:
	Transfomer(Program* prev, Program* next);

private:
	Program* prev;
	Program* next;

};

};
