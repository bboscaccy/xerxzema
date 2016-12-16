#include "Transformer.h"
#include <algorithm>
#include <map>


namespace xerxzema
{

Transformer::Transformer(Program* prev, Program* next): prev(prev), next(next)
{

}

void Transformer::find_new_registers()
{
	for(auto& reg_pair: next->register_listing())
	{
		auto it = prev->register_listing().find(reg_pair.first);
		if(it == prev->register_listing().end())
		{
			new_registers.push_back(reg_pair.second.get());
		}
	}
}

void Transformer::find_deleted_registers()
{
	for(auto& reg_pair: prev->register_listing())
	{
		auto it = next->register_listing().find(reg_pair.first);
		if(it == next->register_listing().end())
		{
			deleted_registers.push_back(reg_pair.second.get());
		}
	}
}

void Transformer::find_type_change_registers()
{
	for(auto& reg_pair: prev->register_listing())
	{
		auto it = next->register_listing().find(reg_pair.first);
		if(it != next->register_listing().end())
		{
			if(it->second->type() != reg_pair.second->type())
				type_change_registers.push_back(it->second.get());
		}
	}
}

void Transformer::parse_registers()
{
	find_new_registers();
	find_deleted_registers();
	find_type_change_registers();
}

};