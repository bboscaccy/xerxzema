#pragma once
#include <type_traits>

#include "Jit.h"
#include "Program.h"
#include "Scheduler.h"
#include "World.h"

namespace xerxzema
{
//TODO better handing of void args and some compile time tests
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
		return do_invoke(std::is_void<R>{}, args...);
	}

	R do_invoke(std::false_type)
	{
		auto r0 = program->output_registers()[0]->offset();
		auto rp0 = (R*)jit->get_state_offset(state, program, r0);
		run_fn();
		return *rp0;
	}

	void do_invoke(std::true_type)
	{
		run_fn();
	}

	template<class T0>
	R do_invoke(std::false_type, const T0& a0)
	{
		auto r0 = program->output_registers()[0]->offset();
		auto n0 = program->input_registers()[0]->offset();
		auto rp0 = (R*)jit->get_state_offset(state, program, r0);
		auto tp0 = (T0*)jit->get_state_offset(state, program, n0);
		*tp0 = a0;
		run_fn();
		return *rp0;
	}

	template<class T0>
	void do_invoke(std::true_type, const T0& a0)
	{
		auto n0 = program->input_registers()[0]->offset();
		auto tp0 = (T0*)jit->get_state_offset(state, program, n0);
		*tp0 = a0;
		run_fn();
	}

	template<class T0, class T1>
	R do_invoke(std::false_type, const T0& a0, const T1& a1)
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

	template<class T0, class T1>
	void do_invoke(std::true_type, const T0& a0, const T1& a1)
	{
		auto n0 = program->input_registers()[0]->offset();
		auto n1 = program->input_registers()[1]->offset();
		auto tp0 = (T0*)jit->get_state_offset(state, program, n0);
		auto tp1 = (T1*)jit->get_state_offset(state, program, n1);
		*tp0 = a0;
		*tp1 = a1;
		run_fn();
	}

	void run_fn()
	{
		auto scheduler = jit->world()->scheduler();
		scheduler->schedule((scheduler_callback)raw_fn, state, 0);
		scheduler->exit_when_empty();
		scheduler->run_async();
		scheduler->wait();
	}

private:
	Jit* jit;
	Program* program;
	void* state;
	void* raw_fn;
};


};
