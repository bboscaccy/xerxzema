#pragma once

#include "Jit.h"
#include "Program.h"

namespace xerxzema
{

template<class R, class... Ts>
class JitInvoke
{
public:
	JitInvoke(Jit* j, Program* p) : jit(j), program(p)
	{
		state = malloc(jit->get_state_size(program));
		memset(state, 0, jit->get_state_size(program));
		raw_fn = jit->get_jitted_function(program);
	}

	~JitInvoke()
	{
		free(state);
	}

	R operator() (const Ts&... args)
	{
		return do_invoke(args...);
	}

	template<class T0>
	R do_invoke(const T0& a0)
	{
		auto r0 = program->output_registers()[0]->offset();
		auto n0 = program->input_registers()[0]->offset();
		auto rp0 = (R*)jit->get_state_offset(state, program, r0);
		auto tp0 = (T0*)jit->get_state_offset(state, program, n0);
		*tp0 = a0;
		run_fn();
		return *rp0;
	}

	template<class T0, class T1>
	R do_invoke(const T0& a0, const T1& a1)
	{
		auto r0 = program->output_registers()[0]->offset();
		auto n0 = program->input_registers()[0]->offset();
		auto n1 = program->input_registers()[1]->offset();
		auto rp0 = (R*)jit->get_state_offset(state, program, r0);
		auto tp0 = (T0*)jit->get_state_offset(state, program, n0);
		auto tp1 = (T1*)jit->get_state_offset(state, program, n1);
		*tp0 = a0;
		*tp1 = a1;
		run_fn();
		return *rp0;
	}

	void run_fn()
	{
		uint64_t (*fn_ptr)(void*);
		fn_ptr = (uint64_t (*)(void*))raw_fn;
		(*fn_ptr)(state);
	}

private:
	Jit* jit;
	Program* program;
	void* state;
	void* raw_fn;
};


};
