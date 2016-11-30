#include "Semantic.h"
#include "Diagnostics.h"
#include "StlUtils.h"

namespace xerxzema
{

HandleTopLevelExpression::HandleTopLevelExpression(Namespace* n) : ns(n)
{
}

void HandleTopLevelExpression::handle_default(xerxzema::Expression *e)
{
	//TODO flag this as invalid
	valid = false;
}

void HandleTopLevelExpression::visit(xerxzema::CodeDefinition *e)
{
	valid = true;
}

HandleCodeDefinitionSignature::HandleCodeDefinitionSignature(Namespace* n, CodeDefinition* d) :
	ns(n), def(d), prog(nullptr), valid(true)
{
}

void HandleCodeDefinitionSignature::handle_default(xerxzema::Expression *e)
{
	//TODO error.
	valid = false;
	if(state == ProcessState::Entry)
	{
		emit_error(e->token.get(), "I was expecting a '->' operator somewhere but instead");
	}
	else
	{
		emit_error(e->token.get(), "malformed definition");
	}
}

void HandleCodeDefinitionSignature::process()
{
	state = ProcessState::Entry;
	def->signature->accept(*this);
}

void HandleCodeDefinitionSignature::visit(BindExpression *e)
{
	if(state == ProcessState::Entry)
	{
		auto lhs = e->lhs.get();
		auto rhs = e->rhs.get();
		state = ProcessState::Lhs;
		lhs->accept(*this);
		state = ProcessState::OutputArgs;
		rhs->accept(*this);
	}
	else
	{
		emit_error(e->token.get(), "malformed definition");
		valid = false;
	}
}

void HandleCodeDefinitionSignature::visit(CallExpression* e)
{
	if(state == ProcessState::Lhs)
	{
		auto target = e->target.get();
		auto args = e->args.get();
		state = ProcessState::Name;
		target->accept(*this);
		state = ProcessState::InputArgs;
		args->accept(*this);
	}
	else
	{
		emit_error(e->token.get(), "malformed definition");
		valid = false;
	}
}

void HandleCodeDefinitionSignature::visit(SymbolExpression* e)
{
	if(state == ProcessState::Name)
	{
		prog = ns->get_program(e->token->data);
	}
	else if(state == ProcessState::GetArgName)
	{
		current_arg_name = e->token->data;
	}
	else if(state == ProcessState::GetArgType)
	{
		current_arg_type = e->token->data;
	}
	else
	{
		emit_error(e->token.get(), "malformed definition");
		valid = false;
	}
}

void HandleCodeDefinitionSignature::visit(xerxzema::ArgListExpression *e)
{
	if(state == ProcessState::InputArgs)
	{
		e->lhs->accept(*this);
		state = ProcessState::InputArgs;
		e->rhs->accept(*this);
	}
	else if(state == ProcessState::OutputArgs)
	{
		e->lhs->accept(*this);
		state = ProcessState::OutputArgs;
		e->rhs->accept(*this);
	}
	else
	{
		emit_error(e->token.get(), "malformed definition");
		valid = false;
	}
}

void HandleCodeDefinitionSignature::visit(xerxzema::AnnotationExpression *e)
{
	if(state == ProcessState::InputArgs || state == ProcessState::OutputArgs)
	{
		current_arg_name = "";
		current_arg_type = "";
		auto last_state = state;
		state = ProcessState::GetArgName;
		e->lhs->accept(*this);
		state = ProcessState::GetArgType;
		e->rhs->accept(*this);

		if(ns->is_type(current_arg_type))
		{
			if(last_state == ProcessState::InputArgs)
				prog->add_input(current_arg_name, ns->type(current_arg_type));
			if(last_state == ProcessState::OutputArgs)
				prog->add_output(current_arg_name, ns->type(current_arg_type));
		}
		else
		{
			valid = false;
			emit_error(e->token.get(), current_arg_type + " is not a valid type");
		}
	}
	else
	{
		valid = false;
		emit_error(e->token.get(), "malformed definition");
	}
}

HandleStatement::HandleStatement(Program* p, Expression* e) : program(p), expr(e), valid(true),
															  counter(0)

{
}

void HandleStatement::process()
{
	expr->accept(*this);
}

void HandleStatement::visit(Statement *e)
{
	HandleExpression handler(program, e->expr.get(), extra_dependencies);
	handler.process();
	counter++;
}

void HandleStatement::visit(StatementBlock *e)
{
	for(auto& s: e->expressions)
	{
		e->accept(*this);
	}
}

void HandleStatement::visit(xerxzema::WithStatement *e)
{
	HandleExpression deps(program, e->with_clause.get());
	deps.process();
	for(auto& d: deps.results())
	{
		extra_dependencies.push_back(d);
	}
	e->statements->accept(*this);
	for(auto& d: deps.results())
	{
		extra_dependencies.pop_back();
	}
}

void HandleStatement::handle_default(xerxzema::Expression *e)
{
	valid = false;
	emit_error(e->token.get(), "this isn't a valid statement.");
}


HandleExpression::HandleExpression(Program* p, Expression* e, const std::vector<RegisterData>& r,
								   const std::vector<RegisterData>& deps) :
	program(p), expr(e), valid(true), result(r), dependencies(deps)
{
}

void HandleExpression::process()
{
	expr->accept(*this);
}

void HandleExpression::visit(xerxzema::SymbolExpression *e)
{
	result.push_back(program->reg_data(e->token->data));
}

void HandleExpression::visit(xerxzema::RealExpression *e)
{
	auto value = atof(e->token->data.c_str());
	result.push_back(program->constant(value));
}

void HandleExpression::visit(xerxzema::ArgListExpression *e)
{
	HandleExpression lhs(program, e->lhs.get(), {}, dependencies);
	lhs.process();
	HandleExpression rhs(program, e->rhs.get(), {}, dependencies);
	rhs.process();
	result = combine_vectors(lhs.result, rhs.result);
}

void HandleExpression::visit(xerxzema::SampleExpression *e)
{
	if(!e->expr->is_a<SymbolExpression>())
	{
		valid = false;
		emit_error(e->expr->token.get(), "This probably isn't sane.");
	}
	HandleExpression child(program, e->expr.get(), {}, dependencies);
	child.process();
	result.push_back(RegisterData({child.result[0].reg, true}));
}

void HandleExpression::visit(xerxzema::AddExpression *e)
{
	HandleExpression lhs(program, e->lhs.get(), {}, dependencies);
	lhs.process();
	HandleExpression rhs(program, e->rhs.get(), {}, dependencies);
	rhs.process();
	valid = rhs.valid && lhs.valid;
	if(result.size() == 0)
		result.push_back(program->temp_reg());
	program->instruction("add", combine_vectors(lhs.result, rhs.result), result, dependencies, e);
}

void HandleExpression::visit(xerxzema::SubExpression *e)
{
	HandleExpression lhs(program, e->lhs.get(), {}, dependencies);
	lhs.process();
	HandleExpression rhs(program, e->rhs.get(), {}, dependencies);
	rhs.process();
	valid = rhs.valid && lhs.valid;
	if(result.size() == 0)
		result.push_back(program->temp_reg());
	program->instruction("sub", combine_vectors(lhs.result, rhs.result), result, dependencies, e);
}

void HandleExpression::visit(xerxzema::MulExpression *e)
{
	HandleExpression lhs(program, e->lhs.get(), {}, dependencies);
	lhs.process();
	HandleExpression rhs(program, e->rhs.get(), {}, dependencies);
	rhs.process();
	valid = rhs.valid && lhs.valid;
	if(result.size() == 0)
		result.push_back(program->temp_reg());
	program->instruction("mul", combine_vectors(lhs.result, rhs.result), result, dependencies, e);
}

void HandleExpression::visit(xerxzema::DivExpression *e)
{
	HandleExpression lhs(program, e->lhs.get(), {}, dependencies);
	lhs.process();
	HandleExpression rhs(program, e->rhs.get(), {}, dependencies);
	rhs.process();
	valid = rhs.valid && lhs.valid;
	if(result.size() == 0)
		result.push_back(program->temp_reg());
	program->instruction("div", combine_vectors(lhs.result, rhs.result), result, dependencies, e);
}

void HandleExpression::visit(xerxzema::BindExpression *e)
{
	HandleExpression rhs(program, e->rhs.get(), {}, dependencies);
	rhs.process();
	result = rhs.result;
	HandleExpression lhs(program, e->lhs.get(), result, dependencies);
	lhs.process();
}

void HandleExpression::visit(xerxzema::GroupExpression *e)
{
	HandleExpression child(program, e->expr.get(), {}, dependencies);
	child.process();
	result = child.result;
}

void HandleExpression::handle_default(xerxzema::Expression *e)
{
	valid = false;
	emit_error(e->token.get(), "Handler Unimplemented");
}


};
