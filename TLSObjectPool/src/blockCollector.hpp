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

	if (head == nullptr) //�ݷ��Ϳ� ��尡 ���� ������ ����
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

		pool->releaseBlocks(releaseHead, releaseTail, chunkCapacity); //temp2�� �������� : relase�� ���ؼ� head�� ���� �ѱ��,
														 //�ؿ��ٿ��� head = temp->next�� �Ѵٸ�, temp�� ��𼱰� �̹� ���Ǿ�����, temp->next�� ������
		size -= chunkCapacity;
	}

	ReleaseSRWLockExclusive(&lock);

	return true;
}

