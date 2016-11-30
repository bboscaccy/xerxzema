#pragma once

#include "Jit.h"

namespace xerxzema
{

template<class R, class T>
R do_invoke(void* fn, void* state, const T& a0)
{
	R res;
	void (*fn_ptr)(void*, const T*, R*);
	fn_ptr = (void (*)(void*, const T*, R*))fn;
	(*fn_ptr)(state, &a0, &res);
	return res;
}

template<class R, class T0, class T1>
R do_invoke(void* fn, void* state, const T0& a0, const T1& a1)
{
	R res;
	void (*fn_ptr)(void*, const T0*, const T1*, R*);
	fn_ptr = (void (*)(void*, const T0*, const T1*, R*))fn;
	(*fn_ptr)(state, &a0, &a1, &res);
	return res;
}

template<class R, class... Ts>
R inner_invoke(void* fn, void* state, const Ts&... args)
{
	return do_invoke<R>(fn, state, args...);
}

template<class R, class... Ts>
R invoke(Jit* jit, const std::string& ns, const std::string& fn, const Ts&... args)
{
	auto fn_ptr = jit->get_jitted_function(ns, fn);
	auto state_size = jit->get_state_size(ns, fn);
	char state[state_size];
	memset(state, 0, state_size);
	return inner_invoke<R>(fn_ptr, state, args...);
}
};
