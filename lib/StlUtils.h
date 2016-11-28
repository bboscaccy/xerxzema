#pragma once

#include <vector>

namespace xerxzema
{
template<class T>
T combine_vectors(const T& lhs, const T& rhs)
{
	T result;
	result.reserve(lhs.size() + rhs.size());
	result.insert(result.end(), lhs.begin(), lhs.end());
	result.insert(result.end(), rhs.begin(), rhs.end());
	return result;
}
};
