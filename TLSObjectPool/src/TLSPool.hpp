#pragma once
#include <stdexcept>

#include "../include/TLSPool.h"
#include "../include/MainPool.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TLSPool class
/// 
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
TLSPool<T>::TLSPool(unsigned int maxChunks, unsigned int allocChunks) : mainPool(MainPool<T>::getInstance()), cache(maxChunks)
{
	info = mainPool.info;
	size = 0;
	head = nullptr;
	tail = nullptr;
	offset = info.chunkCapacity;

#ifdef _DEBUG
	allocSize = 0;
	totalAllocSize = 0;
	totalFreeSize = 0;
	allocChunkSize = 0;
	freeChunkSize = 0;
#endif //_DEBUG

	if (allocChunks > maxChunks)
		allocChunks = maxChunks;

	info.blockThreshold = maxChunks * info.chunkCapacity;
	info.allocChunckUnit = allocChunks;

	allocateBlocks();
}

template <typename T>
TLSPool<T>::~TLSPool()
{
	while (size != 0)
		releaseBlocks();
}


/// <summary>
/// 테스트
/// </summary>
/// <typeparam name="T"> T 임</typeparam>
/// <returns></returns>
template <typename T>
T* TLSPool<T>::alloc()
{
#ifdef _DEBUG
	totalAllocSize++;
#endif //_DEBUG

	memoryBlock<T>* ret = head;
	
	head = head->next;

	if (--offset == 0)
	{
		cache.pop_back();
		offset += info.chunkCapacity;
	}

	if (--size == 0)
	{
		allocateBlocks();
	}

	if (info.mode & MEMORY_FLOW_CHECK)
		ret->key = ENCODEING_MEMORYBLOCK_KEY(info.poolPtr);
	else
		ret->poolPtr = info.poolPtr;

	return (T*)ret;
}
				
template <typename T>
void TLSPool<T>::free(T* var)
{
	memoryBlock<T>* blockPtr = (memoryBlock<T>*)var;

	if (isError(blockPtr))
		throw std::runtime_error("Attempted to return to the wrong object pool");

#ifdef _DEBUG
	totalFreeSize++;
#endif //_DEBUG
	
	blockPtr->next = head;
	head = blockPtr;


	if (++offset >= 1 + info.chunkCapacity)
	{
		offset -= info.chunkCapacity;
		cache.push_back(blockPtr);
	}

	if (++size > info.blockThreshold)
	{
		releaseBlocks();
	}
}


template <typename T>
void TLSPool<T>::allocateBlocks()
{
	for (int i = 0; i < info.allocChunckUnit; i++)
	{
		memoryBlock<T>* blockHead;
		memoryBlock<T>* blockTail;

#ifdef _DEBUG
		allocChunkSize++;
#endif // _DEBUG

		mainPool.newBlocks(blockHead, blockTail);

		if (head != nullptr)
		{
			tail->next = blockHead;
			cache.push_back(tail);
		}
		else
		{
			head = blockHead;
		}

		tail = blockTail;
		tail->next = nullptr;
		size += info.chunkCapacity;
	}
}

template <typename T>
void TLSPool<T>::releaseBlocks()
{
	memoryBlock<T>* nextTail;
	memoryBlock<T>* releaseHead;
	MCCAPACITY		releaseSize;

	if (cache.Peek_front() == nullptr)	// 1개 이상의 청크가 없음, 즉 조각 반환 상태, 남은걸 다 반환할때만 타는 분기
	{
		nextTail = nullptr;
		releaseHead = head;

		for (releaseSize = 1; head != tail; head = head->next, releaseSize++);
	}
	else								// 1개 이상의 청크가 있음, 청크단위 반환 상태
	{
		nextTail = cache.pop_front(); 
		releaseHead = nextTail->next;
		nextTail->next = nullptr;

		releaseSize = info.chunkCapacity;
	}

#ifdef _DEBUG
	freeChunkSize++;
#endif //_DEBUG

	tail->next = nullptr;
	mainPool.releaseBlocks(releaseHead, tail, releaseSize);
	tail = nextTail;
	size -= releaseSize;
}


template <typename T>
bool TLSPool<T>::isError(memoryBlock<T>* var)
{
	if(info.mode & MEMORY_FLOW_CHECK)
	{
		if (flowCheck((memoryBlock<T>*)var))
			return true;
	}

	if (!poolCheck((memoryBlock<T>*)var))
		return true;


	return false;
}

template <typename T>
bool TLSPool<T>::flowCheck(memoryBlock<T>* ptr)
{
	if (GET_MEMORYBLOCK_CORRUPTED_MASK(ptr->key) != CORRUPTED_CHECK_MASK)
		return true;

	return false;
}

template <typename T>
bool TLSPool<T>::poolCheck(memoryBlock<T>* ptr)
{
	if (GET_MEMORYBLOCK_POOL(ptr->key) != (intptr_t)info.poolPtr)
		return false;

	return true;
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 구현간 디버깅용 함수
/// 
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#include <iostream>

template <typename T>
void TLSPool<T>::print()
{
	int count = 0;
	std::cout << "mp " << this << " << ";
	if (head != nullptr)
	{
		for (auto cur = head; cur != tail; cur = cur->next)
		{
			count++;
			std::cout << cur->value << "\t";
		}
	}

	if (tail != nullptr)
	{
		count++;
		std::cout << tail->value << "\t";
	}

	std::cout << " (" << count << "/" << this->size << ")";

	std::cout << std::endl;
}
#endif //_DEBUG