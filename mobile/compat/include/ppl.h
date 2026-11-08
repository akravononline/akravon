// Cynex: msvc ppl - parallel_for / parallel_for_each (simdilik sirali)
#pragma once
#include <iterator>
namespace concurrency
{
	template <class Index, class Func>
	inline void parallel_for(Index first, Index last, const Func& f)
	{
		for (Index i = first; i < last; ++i)
			f(i);
	}

	template <class Index, class Func>
	inline void parallel_for(Index first, Index last, Index step, const Func& f)
	{
		for (Index i = first; i < last; i += step)
			f(i);
	}

	template <class It, class Func>
	inline void parallel_for_each(It first, It last, const Func& f)
	{
		for (; first != last; ++first)
			f(*first);
	}
}
namespace Concurrency = concurrency;
