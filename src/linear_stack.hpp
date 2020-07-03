#pragma once

#include <cstdint>
#include <stdexcept>

namespace rt {

/**
	A simple stack container allocated on the stack (and not on the heap)
*/
template <typename T, unsigned int N>
class linear_stack
{
public:
	template <typename... Args>
	T &emplace(Args&&... args)
	{
		if (m_top_ptr - reinterpret_cast<T*>(m_data) == N)
			throw std::overflow_error("linear_stack overflow");
		
		new(m_top_ptr++) T(std::forward<Args>(args)...);
		return *(m_top_ptr - 1);
	}

	template <typename U>
	T &push(U &&u)
	{
		if (m_top_ptr - reinterpret_cast<T*>(m_data) == N)
			throw std::overflow_error("linear_stack overflow");

		new(m_top_ptr++) T(std::forward<U>(u));
		return *(m_top_ptr - 1);
	}

	void pop()
	{
		if (empty())
			throw std::underflow_error("linear_stack - pop() called on empty stack");
		(--m_top_ptr)->~T();
	}

	T &top()
	{
		if (empty())
			throw std::out_of_range("linear_stack - top() called on empty stack");
		return *(m_top_ptr - 1);
	}

	bool empty() const
	{
		return m_top_ptr == reinterpret_cast<const T*>(m_data);
	}

private:
	T *m_top_ptr = reinterpret_cast<T*>(m_data);
	std::uint8_t m_data[N * sizeof(T)];
};

}