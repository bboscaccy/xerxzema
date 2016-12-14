#include "Session.h"
#include "World.h"
#include "Lexer.h"
#include "Parser.h"
#include "Diagnostics.h"
#include "Semantic.h"
#include <sstream>

namespace xerxzema
{
Session::Session(World* world):world(world)
{

}

void Session::eval(const std::string& str, const std::string& ns)
{
	auto parent = world->get_namespace(ns);
	std::stringstream ss;
	ss << str;
	Lexer lexer(ss);

	while(lexer.peek()->type != TokenType::Eof)
	{
		auto expr = expression(lexer);
		HandleTopLevelExpression sema(parent);
		expr->accept(sema);
	}
	auto program = parent->get_default_program();
	world->jit()->compile_namespace(parent);

	//start the scheduler if it's not running?
	//the malloc'd buffer will change size...
	//so nuke it for now and this is a todo...
	auto state = malloc(world->jit()->get_state_size(program));
	memset(state, 0, world->jit()->get_state_size(program));
	auto raw_fn = world->jit()->get_jitted_function(program);
	world->scheduler()->schedule((scheduler_callback)raw_fn, state, now());
}

};
