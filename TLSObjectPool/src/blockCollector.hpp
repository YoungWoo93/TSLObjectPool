#pragma once
#include <Windows.h>

#include "../include/MainPool.h"



template <typename T>
MainPool<T>::blockCollector::blockCollector(MCCAPACITY capacity, MainPool<T>* mainPool)
{
	size = 0;
	chunkCapacity = capacity;
	head = nullptr;
	tail = nullptr;
	pool = mainPool;
	lock = SRWLOCK_INIT;

	chunkTail = nullptr;
}

template <typename T>
MainPool<T>::blockCollector::~blockCollector()
{
	memoryBlock<T>* next;

	while (size-- != 0)
	{
		next = head;
		next = next->next;
		delete head;
		head = next;
	}
}

template <typename T>
bool MainPool<T>::blockCollector::collect(memoryBlock<T>* _head, memoryBlock<T>* _tail, MCCAPACITY _size)
{
	AcquireSRWLockExclusive(&lock);
	if (head == nullptr) //콜렉터에 노드가 전혀 없을때 진입
	{
		head = _head;
		tail = head;
		size++;
		_size--;
	}
	else
	{
		tail->next = _head;
	}
	
	for (unsigned int i = 0; i < _size; i++)
	{
		if (size++ >= chunkCapacity)
		{
			chunkTail = tail;
			tail = tail->next;
			chunkTail->next = nullptr;
			pool->releaseBlocks(head, chunkTail, chunkCapacity);
			head = tail;
			size -= chunkCapacity;
		}
		else
		{
			tail = tail->next;
		}
	}
	ReleaseSRWLockExclusive(&lock);
	return true;
}

