#ifndef QUEUE_H_
#define QUEUE_H_

/**
* queue for data struct
* create by lingo 2015-2-26
*/
template<class T>
class Queue{
public:
	Queue();
	~Queue();

	/**
	* push element to queue
	* @param t1
	* @return
	*/
	void push(T t1);
	/**
	* delete head of queue
	* @return
	*/
	int pop();
	/**
	* take element of head for queue
	* @return T
	*/
	T queue();
	/**
	* check queue
	* @return
	*/
	bool is_empty();
	/**
	* clear all of queue
	* @return
	*/
	void clear();
	/**
	* output all element
	* @return
	*/
	void output();
	/**
	* get lenght of queue
	*/
	int size();
private:
	int m_size;

	struct List{
		T element;
		List* next;
	};

	struct List *m_head;
	struct List *m_tail;
};

#endif