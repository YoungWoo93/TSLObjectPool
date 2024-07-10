#pragma once
#include <stdexcept>

#include "../include/TLSPool.h"
#include "../include/MainPool.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TLSPool class
/// 
///	������Ʈ Ǯ�� �̿��Կ� �־�, ���� ���μ��� �Ŀ��� ���� �ҿ�Ǵ� ������ �ʱ� �Ҵ� �ܰ��� �����ߴ�.
///		-> �ʱ� �Ҵ��� ������Ʈ Ǯ ��ü�� �����ϴ� �Ϳ�, ����� ���� �޸𸮸� �̸� Ȯ���ϴ� ������ �߰���.
/// 
/// �׷��ٰ� �����带 �ѹ� �����Ҷ�, ���� �Լ��� ������Ʈ Ǯ ��ü�� �����Ѵٸ� ���� �������� �������� �ʷ��ȴ�.
///		-> Ư�� �Լ��� ������ ������Ʈ Ǯ�� �����Ѵٸ� �ش� �����忡�� ȣ��Ǵ� ���� �Լ��� ��� �ش� ��ü�� ���ڷ� �Ѱܾ� �Ѵ�.
/// 
/// ���Ͱ��� �������� ���� ���ؼ��� ���� ������ �����ϴ� ���� ���� ����������, �׷��� �Ǹ� ��� �����尡 �ϳ��� ������ �ٶ󺸰� �ȴ�.
///		-> �޸�Ǯ�� �ʱ� ������ �ϳ��� �����尣 ������ ���̱� �����.
///		-> ������ ������ �������� ������ ��������ó�� ���� �����ϵ�, �����帶�� �������� ������ threrad local storage�� �̿��Ѵ�.
/// 
/// ������ TLS�� ������ ������ ����� ������Ʈ Ǯ ��ü�� �ø� ���� ����
///		-> �������� �Ҵ��ϰ� �ش� �ּҸ� �ø���.
///		-> �̷��� �Ǹ� �����尡 ����Ǳ� �� TLS�� ����� ������Ʈ Ǯ�� ���� �޸� ȸ���� �ʿ��ϴ�.
///		-> �̺κ��� �ڵ��� ���ֱ� ���ؼ� �������� �����Լ��� ���������� ��ü�� ������ ������ �����ֱ⸦ �̿��ϴ°��� ���� ���� �� ����.
///			-> ���� �����Լ��� �ƴѰ����� �������� ��ü�� ���� �� ���, ���� ī��Ʈ�� �̿��ؾ� �Ѵ�. (�ٸ� �Լ��� ȣ���ϰ� �Դ��� Ǯ�� ����� �� ����)
///			-> ������ ���� ī��Ʈ�� �̿��Ѵ� �Ͽ��� ���ǹ��� Ǯ�� �ʱ��Ҵ��� ���� �߻� �� �� �ִ�.
///		-> �� ������ ����, ������ �ѹ� �����ϰ� ���� �Լ����� ó�� �ϴ� ����� ���� �� �ִ�.
///			-> �̰�� ���ο� Ÿ���� �޸�Ǯ�� ��� �Ϸ� �� �� ���� ���� �Լ��� �����ؾ��ϴ� �������� ���� �� �ִ�.
///			-> ���� �Լ������� pool manager�� TLS�� ����, �����ϵ��� ����
///			-> �׸��� TLSPool�� ���� �Ҷ� pool manager�� ������ִ� ���¸� ������.
/// 
/// TLS �������� �޸� ��� ������ ���� ���·� (ĳ�� ���߷��� �ø��� ����)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



template <typename T>
TLSPool<T>::TLSPool(unsigned int maxChunks, unsigned int allocChunks) : mainPool(MainPool<T>::getInstance()), cache(maxChunks)
{
	if (allocChunks > maxChunks)
		allocChunks = maxChunks;

	size = 0;
	head = nullptr;
	tail = nullptr;
	offset = mainPool.info.chunkCapacity;
	info = mainPool.info;
	info.blockThreshold = maxChunks * info.chunkCapacity;
	info.allocChunckUnit = allocChunks;

#ifdef TLSPOOL_DEBUG
	allocSize = 0;
	totalAllocSize = 0;
	totalFreeSize = 0;
	allocChunkSize = 0;
	freeChunkSize = 0;
#endif //TLSPOOL_DEBUG

	allocateBlocks();
}

template <typename T>
TLSPool<T>::~TLSPool()
{
	while (size != 0)
		releaseBlocks();
}


/// <summary>
/// TLSPool ���� ����� �޸𸮸� ��ȯ. 
///	(momoryBlock�� ���� �ּҿ� ������ �����ּҰ� ����.)
/// </summary>
/// <returns>��밡���� TŸ�� �޸��� �ּ�</returns>
template <typename T>
T* TLSPool<T>::alloc()
{
	memoryBlock<T>* ret = head;
	
	head = head->next;

	if (--offset == 0)
	{
		cache.pop_back();
		offset += info.chunkCapacity;
	}
	if (--size == 0)
		allocateBlocks();

	ret->poolPtr = info.poolPtr;

	if (info.mode & MEMORY_CORRUPTED_CHECK)
		ret->key = ENCODEING_MEMORYBLOCK_KEY(ret->poolPtr);

		

#ifdef TLSPOOL_DEBUG
	totalAllocSize++;
#endif //TLSPOOL_DEBUG

	return (T*)ret;
}
	
/// <summary>
/// ����� ���� �޸𸮸� TLSPool ���� ����.
/// </summary>
/// <param name="var">����� ���� �޸�</param>
template <typename T>
void TLSPool<T>::free(T* var)
{
	memoryBlock<T>* blockPtr = (memoryBlock<T>*)var;

	if (isError(blockPtr))
		throw std::runtime_error("Attempted to return to the wrong object pool");

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

#ifdef TLSPOOL_DEBUG
	totalFreeSize++;
#endif //TLSPOOL_DEBUG
}

/// <summary>
/// TLSPool ���� �޸� ����� ���ڶ� ��� mainPool�κ��� ���ο� �޸� ûũ�� �޾ƿ��� ����
/// </summary>
template <typename T>
void TLSPool<T>::allocateBlocks()
{
	for (int i = 0; i < info.allocChunckUnit; i++)
	{
		memoryBlock<T>* blockHead;
		memoryBlock<T>* blockTail;

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
		size += info.chunkCapacity;

#ifdef TLSPOOL_DEBUG
		allocChunkSize++;
#endif // TLSPOOL_DEBUG
	}
}

/// <summary>
/// TLS ���� �޸� ����� �ʿ� �̻����� ���� ��� mainPool�� ��ȯ�ϴ� ����.
/// </summary>
template <typename T>
void TLSPool<T>::releaseBlocks()
{
	memoryBlock<T>* nextTail;			// ������ ���� �ٴں��� 1�� ûũ ũ��(n��)�� ����� ��ȯ�Ѵ�.
	memoryBlock<T>* releaseHead;		// ������ ������(tail) �� ����ǹǷ� ����� tail (= nextTail)�� �غ��ؾ��Ѵ�.
	MCCAPACITY		releaseSize;		// ���� tail���� n�� ������ ��ġ (release �� ����Ʈ�� head) ���� �غ��ؾ��Ѵ�.

	if (cache.peek_front() != nullptr)	// 1�� �̻��� ĳ�õ� ûũ�� ����, ûũ���� ��ȯ ����
	{
		nextTail = cache.pop_front(); 
		releaseHead = nextTail->next;
		nextTail->next = nullptr;

		releaseSize = info.chunkCapacity;
	}
	else								// 1�� �̻��� ĳ�õ� ûũ�� ����, �� ���� ��ȯ ����
	{									// ������ �Ҹ�������������� �� ��ȯ�Ҷ��� Ÿ�� �б�
		nextTail = nullptr;
		releaseHead = head;

		for (releaseSize = 1; head != tail; head = head->next, releaseSize++);
	}

	mainPool.releaseBlocks(releaseHead, tail, releaseSize);
	tail = nextTail;
	size -= releaseSize;

#ifdef TLSPOOL_DEBUG
	freeChunkSize++;
#endif //TLSPOOL_DEBUG
}

/// <summary>
/// ��忡 ���� ���� üũ
/// </summary>
/// <returns>true = error</returns>
template <typename T>
bool TLSPool<T>::isError(memoryBlock<T>* var)
{
	if(info.mode & MEMORY_CORRUPTED_CHECK)
	{
		if (corruptedCheck((memoryBlock<T>*)var))
			return true;
	}
	if (!poolCheck((memoryBlock<T>*)var))
		return true;

	return false;
}

/// <summary>
/// ���� �������� ������� �ʴ� �޸𸮸� �̿��Ͽ� �޸� �����÷ο� üũ.
/// true = error
/// </summary>
/// <returns>true = error</returns>
template <typename T>
bool TLSPool<T>::corruptedCheck(memoryBlock<T>* ptr)
{
	if (GET_MEMORYBLOCK_CORRUPTED_MASK(ptr->key) != CORRUPTED_CHECK_MASK)	// �̸� ����ŷ�ص� key ���� �����Ǿ��� Ȯ���Ѵ�.
		return true;
	
	return false;
}

/// <summary>
/// ��ȯ�� �޸𸮰� �ش� Ǯ(���� Ǯ)���� ���� �޸𸮰� �´��� üũ.
/// false = error
/// </summary>
/// <returns>false = error</returns>
template <typename T>
bool TLSPool<T>::poolCheck(memoryBlock<T>* ptr)
{
	if (GET_MEMORYBLOCK_POOL(ptr->poolPtr) != (intptr_t)info.poolPtr)		// �̸� �����ص� mainPool�� ���� �³� Ȯ���Ѵ�.
		return false;

	return true;
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ������ ������ �Լ�
/// 
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef TLSPOOL_DEBUG
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
#endif //TLSPOOL_DEBUG