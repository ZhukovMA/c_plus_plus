///////////////////////////////////////////////////////////////////////////////////

// chapter : Memory

///////////////////////////////////////////////////////////////////////////////////

// section : Iterators

///////////////////////////////////////////////////////////////////////////////////

// content : List Iterators
//
// content : Operator ->
//
// content : Range-Based Statement for

///////////////////////////////////////////////////////////////////////////////////

#include <iterator>
#include <memory>
#include <utility>

///////////////////////////////////////////////////////////////////////////////////

template < typename T > class List
{
private :

	struct Node
	{
		T x = T();

		std::shared_ptr < Node > next;
		std::weak_ptr   < Node > prev;

		Node() = default;

		Node(T value) : x(std::move(value)) {}
	};

public :

	class Iterator
	{
	public :

		using iterator_category = std::bidirectional_iterator_tag;
		using value_type        = T;
		using difference_type   = std::ptrdiff_t;
		using pointer           = T *;
		using reference         = T &;

	//  -------------------------------------------------------------------

		Iterator(
			std::shared_ptr < Node > node = nullptr,
			std::shared_ptr < Node > tail = nullptr
		)
			: m_node(node), m_tail(tail) {}

	//  -------------------------------------------------------------------

		auto const operator++(int)
		{
			auto x = *this;

			if (m_node)
			{
				m_node = m_node->next;
			}

			return x;
		}

	//  -------------------------------------------------------------------

		auto & operator++()
		{
			if (m_node)
			{
				m_node = m_node->next;
			}

			return *this;
		}

	//  -------------------------------------------------------------------

		auto const operator--(int)
		{
			auto x = *this;

			if (m_node)
			{
				m_node = m_node->prev.lock();
			}
			else
			{
				m_node = m_tail;
			}

			return x;
		}

	//  -------------------------------------------------------------------

		auto & operator--()
		{
			if (m_node)
			{
				m_node = m_node->prev.lock();
			}
			else
			{
				m_node = m_tail;
			}

			return *this;
		}

	//  -------------------------------------------------------------------

		auto & operator* () const { return  m_node->x; }

		auto   operator->() const { return &m_node->x; }

	//  -------------------------------------------------------------------

		friend auto operator==(Iterator const & lhs, Iterator const & rhs)
		{
			return lhs.m_node == rhs.m_node;
		}

		friend auto operator!=(Iterator const & lhs, Iterator const & rhs)
		{
			return !(lhs == rhs);
		}

	private :

		std::shared_ptr < Node > m_node;
		std::shared_ptr < Node > m_tail;
	};


//  -----------------------------------------------------------------------

	auto begin() const { return Iterator(m_head, m_tail); }

	auto end  () const { return Iterator(nullptr, m_tail); }

//  -----------------------------------------------------------------------

	void push_back(T x)
	{
		auto node = std::make_shared < Node > (std::move(x));

		if (m_tail)
		{
			node->prev = m_tail;
			m_tail->next = node;
			m_tail = node;
		}
		else
		{
			m_head = node;
			m_tail = node;
		}
	}

private :

	std::shared_ptr < Node > m_head;
	std::shared_ptr < Node > m_tail;
};

///////////////////////////////////////////////////////////////////////////////////

int main()
{
	List < int > list;

//  -------------------------------------------------------------------------------

	list.push_back(1);
	list.push_back(2);
	list.push_back(3);

//  -------------------------------------------------------------------------------

	for (auto iterator = std::begin(list); iterator != std::end(list); ++iterator);

//  -------------------------------------------------------------------------------

	for ([[maybe_unused]] auto element : list);

//  -------------------------------------------------------------------------------

	auto iterator = std::end(list);
	--iterator;
	iterator--;
}

///////////////////////////////////////////////////////////////////////////////////