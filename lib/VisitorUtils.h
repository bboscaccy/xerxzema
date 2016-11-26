#pragma once

namespace xerxzema
{
namespace util
{
template<class V, class B, class T>
class is_a_impl : public V
{
public:
	void visit(T* t)
	{
		valid = true;
	}
	void handle_default(B* b)
	{
		valid = false;
	}
	bool valid;
};

template<class V, class B, class T>
class as_a_impl : public V
{
public:
	as_a_impl() : value(nullptr)
	{
	}

	void visit(T* t)
	{
		value = t;
	}
	void handle_default(B* b)
	{
	}
	T* value;
};

template<class V, class B, class T>
bool is_a(B* b)
{
	is_a_impl<V, B, T> impl;
	b->accept(impl);
	return impl.valid;
}

template<class V, class B, class T>
T* as_a(B* b)
{
	as_a_impl<V, B, T> impl;
	b->accept(impl);
	return impl.value;
}
};
};
