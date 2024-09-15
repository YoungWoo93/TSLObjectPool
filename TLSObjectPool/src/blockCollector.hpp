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
	memoryBlock<T>* next = head;

	while (next != nullptr)
	{
		head = next->next;
		delete next;
		next = head;
	}
}

template <typename T>
bool MainPool<T>::blockCollector::collect(memoryBlock<T>* _head, memoryBlock<T>* _tail, MCCAPACITY _size)
{
	AcquireSRWLockExclusive(&lock);

	if (head == nullptr) //콜렉터에 노드가 전혀 없을때 진입
		head = _head;
	else
		tail->next = _head;
	

	tail = _tail;
	size += _size;

	if (size >= chunkCapacity)
	{
		memoryBlock<T>* releaseHead = head;
		memoryBlock<T>* releaseTail = head;

		for (MCCAPACITY i = 1; i < chunkCapacity; i++)
			releaseTail = releaseTail->next;

		
		head = releaseTail->next;
		releaseTail->next = nullptr;

		pool->releaseBlocks(releaseHead, releaseTail, chunkCapacity); //temp2를 쓰는이유 : relase를 위해서 head를 먼저 넘기고,
														 //밑에줄에서 head = temp->next를 한다면, temp가 어디선가 이미 사용되었을때, temp->next가 오염됨
		size -= chunkCapacity;
	}

	ReleaseSRWLockExclusive(&lock);

	return true;
}

