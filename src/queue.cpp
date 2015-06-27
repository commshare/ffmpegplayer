#include "queue.h"
#include <iostream>
#include <assert.h>

using namespace std;

template<class T>
Queue<T>::Queue():m_size(0){
	List* temp = new List;
	temp->next = NULL;
	this->m_head = this->m_tail = temp;
}

template<class T>
Queue<T>::~Queue(){
	this->clear();
}

template<class T>
void Queue<T>::push(T t1){
	if(this->m_tail == NULL){
		return ;
	}

	List* temp = new List;
	temp->element = t1;
	temp->next = NULL;

	this->m_tail->next = temp;
	this->m_tail = temp;

	this->m_size += 1;
}

template<class T>
void Queue<T>::pop(){
	if(this->m_size == 0 ||this->m_head == this->m_tail){
		return ;
	}
	if(this->m_head->next == NULL){
		return ;
	}

	List* temp = this->m_head->next;
	this->m_head->next = temp->next;
	delete temp;
	
	this->m_size -= 1;
}

template<class T>
T Queue<T>::queue(){
	if(this->m_head == NULL || this->m_head->next == NULL){
		return T();
	}

	return this->m_head->next->element;
}

template<class T>
bool Queue<T>::is_empty(){
	if(this->m_size <= 0){
		return true;
	}

	return false;
}

template<class T>
void Queue<T>::clear(){
	if(this->m_head == NULL || this->m_head->next == NULL){
		return ;
	}

	List* temp = this->m_head->next;
	while(temp != NULL){
		List* te = temp->next;
		delete temp;
		temp = te;
	}
	this->m_tail = this->m_head;

	this->m_size = 0;
}

template<class T>
void Queue<T>::output(){
	if(this->m_size == 0 || this->m_head == NULL || this->m_head->next == NULL){
		return ;
	}

	List* temp = this->m_head->next;

	while(temp != NULL){
		cout<<temp->element<<"\t";
		temp = temp->next;
	}
		
	cout<<endl;
}


template<class T>
int Queue<T>::size(){
	return this->m_size;
}