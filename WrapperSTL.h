
#if defined( __NEED_CONCURRENT_QUEUE__ )

#include <condition_variable>
#include <mutex>
#include <queue>


/** @class concurrent::queue
 **
 ** Contains the following methods:
 ** void clear()
 ** bool empty()
 ** void push( T const & data )
 ** void pop( T & output )
 ** bool try_pop( T & output )
 ** void wait_pop( T & output )
 **/
namespace concurrent {
template<typename T>
class queue
{
private:
	typedef struct SNode {
		struct SNode	*next,
						*prev;
		T				element;
	} TNode;

	TNode			*n_front,
					*n_back;
	size_t			n_count;
    std::mutex		the_mutex;
    std::condition_variable the_condition_variable;

public:

	queue()
	{
		std::unique_lock<std::mutex> lock(the_mutex);

		this->n_front = nullptr;
		this->n_back = nullptr;
		this->n_count = 0;
	}
	~queue()
	{
		std::unique_lock<std::mutex> lock(the_mutex);

		if ( n_front != nullptr && n_back != nullptr && n_count > 0 )
		{
			TNode *node = n_front;

			while ( node != nullptr )
			{
				TNode *node_ptr = node;

				node = node->next;

				delete node_ptr;
			}
		}

	}

	void clear()
	{
		std::unique_lock<std::mutex> lock(the_mutex);

		if ( n_front != nullptr && n_back != nullptr )
		{
			TNode *node = n_front;

			while ( node != nullptr )
			{
				TNode *node_ptr = node;

				node = node->next;

				delete node_ptr;
			}
		}

		n_front = nullptr;
		n_back = nullptr;
		n_count = 0;
	}

    void push(T const& data)
    {
		std::unique_lock<std::mutex> lock(the_mutex);

		// -- Allocate a new node and initialise it
		TNode *new_node		= new TNode;
		new_node->element	= data;
		new_node->next		= nullptr;
		new_node->prev		= nullptr;

		if ( n_front == nullptr && n_back == nullptr )
		{
			// -- The new node is the front and back of the queue
			n_front = new_node;
			n_back = new_node;
		}
		else
		{
			// -- The new node is behind the current back-of-the-line node.
			new_node->prev = n_back;
			// -- The current back-of-the-line-node is in front of the new node.
			n_back->next = new_node;
			// -- The back of the line is now new_node;
			n_back = new_node;
		}

		// -- Increment the counter, we have a new node
		this->n_count++;

		lock.unlock();
		the_condition_variable.notify_one();
    }

	size_t size()
	{
		std::lock_guard<std::mutex> lock(the_mutex);

		return this->n_count;
	}

    bool empty()
    {
        std::lock_guard<std::mutex> lock(the_mutex);

        if ( n_front == nullptr && n_back == nullptr )
			return true;
		else
			return false;
    }

	T & front(void)
	{
		std::unique_lock<std::mutex> lock(the_mutex);
		return this->n_front->element;
	}

    void pop()
    {
    	if ( !empty() )
		{
			std::unique_lock<std::mutex> lock(the_mutex);

			if ( n_front == n_back )
			{
				delete n_front;

				n_front = nullptr;
				n_back = nullptr;
				n_count = 0;
			}
			else
			{
				// -- Reassign pointers
				TNode *new_front = n_front->next;
				n_front->next->prev = nullptr;

				// -- Deallocate the front node
				delete n_front;

				// -- Assign the new front node's pointer
				n_front = new_front;

				// -- Decrement the count
				n_count--;
			}
		}
    }

    bool try_pop(T& popped_value)
    {
		//std::lock_guard<std::mutex> lock(the_mutex);

		if ( empty() )
		{
			return false;
		}
		else
		{
			// -- Get the stored value
			popped_value = n_front->element;

			if ( n_front == n_back )
			{
				delete n_front;

				n_front = nullptr;
				n_back = nullptr;
				n_count = 0;
			}
			else
			{
				// -- Reassign pointers
				TNode *new_front = n_front->next;
				n_front->next->prev = nullptr;

				// -- Deallocate the front node
				delete n_front;

				// -- Assign the new front node's pointer
				n_front = new_front;

				// -- Decrement the count
				n_count--;
			}
		}

        return true;
    }

    void wait_and_pop(T& popped_value)
    {
        std::unique_lock<std::mutex> lock(the_mutex);

        while ( empty() )
        {
            the_condition_variable.wait(lock);
        }

		// -- Get the stored value
		popped_value = n_front->element;

		if ( n_front == n_back )
		{
			delete n_front;

			n_front = nullptr;
			n_back = nullptr;
			n_count = 0;
		}
		else
		{
			// -- Reassign pointers
			TNode *new_front = n_front->next;
			n_front->next->prev = nullptr;

			// -- Deallocate the front node
			delete n_front;

			// -- Assign the new front node's pointer
			n_front = new_front;

			// -- Decrement the count
			n_count--;
		}
    }

};
}// namespace concurrent

#endif // defined
