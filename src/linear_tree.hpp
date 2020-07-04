#pragma once

#include <vector>
#include <optional>
#include <cmath>
#include <stdexcept>
#include <iostream>

namespace rt {

/**
	Tree data structure, organized as an array to minimize cache miss rate
*/
template <typename T>
class linear_tree
{
public:
	using vector_iterator = typename std::vector<T>::iterator;
	class iterator;
	friend class iterator;

	linear_tree(int initial_height = 4)
	{
		set_height(initial_height);
	}

	iterator get_root_node()
	{
		return iterator{this, 0};
	}

	const iterator get_root_node() const
	{
		return iterator{const_cast<linear_tree<T>*>(this), 0};
	}

	/**
		Sets height of the tree (equivalent of resize)
	*/
	void set_height(int height)
	{
		if (height > 25)
			throw std::overflow_error("linear_tree overflow - just for safety");

		m_height = height;
		m_nodes.resize(1 << (height - 1));
	}

	/**
		Grows one layer
	*/
	void grow_layer()
	{
		set_height(m_height + 1);
	}

	/**
		Returns current height of the tree
	*/
	int get_height() const
	{
		return m_height;
	}

private:
	int m_height;
	std::vector<std::optional<T>> m_nodes;
};



template <typename T>
class linear_tree<T>::iterator
{
	friend class linear_tree<T>;

public:
	iterator() = default;

	iterator(linear_tree<T> *tree, unsigned int index) :
		m_tree(tree),
		m_index(index)
	{
	}

	int get_layer() const
	{
		return std::log2(m_index + 1);
	}

	iterator parent()
	{
		return iterator{m_tree, (m_index - 1) / 2};
	}

	const iterator parent() const
	{
		return iterator{m_tree, (m_index - 1) / 2};
	}

	bool has_parent() const
	{
		return parent().has_value();
	}

	iterator left()
	{
		return iterator{m_tree, m_index * 2 + 1};
	}

	const iterator left() const
	{
		return iterator{m_tree, m_index * 2 + 1};
	}

	bool has_left() const
	{
		return left().has_value();
	}

	iterator right()
	{
		return iterator{m_tree, m_index * 2 + 2};
	}

	const iterator right() const
	{
		return iterator{m_tree, m_index * 2 + 2};
	}

	bool has_right() const
	{
		return right().has_value();
	}

	bool has_value() const
	{
		return (m_index < m_tree->m_nodes.size()) && m_tree->m_nodes[m_index].has_value();
	}

	/**
		Returns true if this node can be emplaced.
		Only nodes that:
			a) already have a value
			b) have a parent
			c) are the root node
		can be emplaced
	*/
	bool can_emplace() const
	{
		return has_value() || has_parent() || m_index == 0;
	} 

	/**
		Constructs T inside the node
	*/
	template <typename... Args>
	T &emplace(Args&&... args)
	{
		if (can_emplace())
		{
			// If more than one layer would be required to grow,
			// can_emplace() would have returned false (lack of parent)
			if (m_index >= m_tree->m_nodes.size())
				m_tree->grow_layer();


			m_tree->m_nodes.at(m_index).emplace(std::forward<Args>(args)...);
			return m_tree->m_nodes[m_index].value();
		}
		else
			throw std::out_of_range("emplace() called on linear_tree node that can't be emplaced");
	}

	/**
		Destroys T contained in this node and all children nodes too
	*/
	void remove()
	{
		if (has_left()) left().remove();
		if (has_right()) right().remove();
		m_tree->m_nodes[m_index].reset();
	}

	/**	
		Access to the contained T
	*/
	T &operator*()
	{
		return m_tree->m_nodes[m_index].value();		
	}

	const T &operator*() const
	{
		return m_tree->m_nodes[m_index].value();
	}

	T *operator->()
	{
		return &m_tree->m_nodes[m_index].value();
	}

	const T *operator->() const
	{
		return &m_tree->m_nodes[m_index].value();
	}

	unsigned int get_tree_index() const
	{
		return m_index;
	}

private:

	linear_tree *m_tree;
	unsigned int m_index;
};

}