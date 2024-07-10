#pragma once
#include <stdexcept>

#include "../include/TLSPool.h"
#include "../include/MainPool.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TLSPool class
/// 
///	오브젝트 풀을 이용함에 있어, 가장 프로세스 파워가 많이 소요되는 시점은 초기 할당 단계라고 생각했다.
///		-> 초기 할당은 오브젝트 풀 객체를 생성하는 것에, 충분한 여분 메모리를 미리 확보하는 동작이 추가됨.
/// 
/// 그렇다고 스레드를 한번 생성할때, 진입 함수에 오브젝트 풀 객체를 생성한다면 개발 과정에서 불편함이 초래된다.
///		-> 특정 함수의 지역에 오브젝트 풀이 존재한다면 해당 스레드에서 호출되는 많은 함수에 모두 해당 객체를 인자로 넘겨야 한다.
/// 
/// 위와같은 불편함을 막기 위해서는 전역 변수로 선언하는 것이 가장 간단하지만, 그렇게 되면 모든 스레드가 하나의 변수를 바라보게 된다.
///		-> 메모리풀의 초기 목적중 하나인 스레드간 경합을 줄이기 힘들다.
///		-> 동일한 스레드 내에서는 언제나 전역변수처럼 접근 가능하되, 스레드마다 동립적인 공간인 threrad local storage를 이용한다.
/// 
/// 하지만 TLS의 공간이 협소한 관계로 오브젝트 풀 전체를 올릴 수는 없다
///		-> 동적으로 할당하고 해당 주소만 올린다.
///		-> 이렇게 되면 스레드가 종료되기 전 TLS에 저장된 오브젝트 풀에 대한 메모리 회수가 필요하다.
///		-> 이부분을 자동을 해주기 위해선 스레드의 진입함수에 지역변수로 객체를 생성해 변수의 생명주기를 이용하는것이 제일 편리할 것 같다.
///			-> 만약 진입함수가 아닌곳에서 지역으로 객체를 생성 할 경우, 참조 카운트를 이용해야 한다. (다른 함수를 호출하고 왔더니 풀이 사라질 수 있음)
///			-> 하지만 참조 카운트를 이용한다 하여도 무의미한 풀의 초기할당이 많이 발생 할 수 있다.
///		-> 즉 스레드 생성, 삭제를 한번 래핑하고 래핑 함수에서 처리 하는 방법이 있을 수 있다.
///			-> 이경우 새로운 타입의 메모리풀을 사용 하려 할 때 마다 래핑 함수를 수정해야하는 불편함이 있을 수 있다.
///			-> 래핑 함수에서는 pool manager를 TLS에 생성, 삭제하도록 하자
///			-> 그리고 TLSPool을 생성 할때 pool manager에 등록해주는 형태를 취하자.
/// 
/// TLS 내에서의 메모리 블록 관리는 스택 형태로 (캐시 적중률을 올리기 위함)
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
/// TLSPool 내에 저장된 메모리를 반환. 
///	(momoryBlock의 시작 주소와 변수의 시작주소가 같다.)
/// </summary>
/// <returns>사용가능한 T타입 메모리의 주소</returns>
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
/// 사용이 끝난 메모리를 TLSPool 내에 저장.
/// </summary>
/// <param name="var">사용이 끝난 메모리</param>
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
/// TLSPool 내에 메모리 블록이 모자란 경우 mainPool로부터 새로운 메모리 청크를 받아오는 동작
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
/// TLS 내에 메모리 블록이 필요 이상으로 있을 경우 mainPool에 반환하는 동작.
/// </summary>
template <typename T>
void TLSPool<T>::releaseBlocks()
{
	memoryBlock<T>* nextTail;			// 스택의 가장 바닥부터 1개 청크 크기(n개)의 블록을 반환한다.
	memoryBlock<T>* releaseHead;		// 스택의 마지막(tail) 이 변경되므로 변경될 tail (= nextTail)을 준비해야한다.
	MCCAPACITY		releaseSize;		// 또한 tail부터 n개 떨어진 위치 (release 될 리스트의 head) 또한 준비해야한다.

	if (cache.peek_front() != nullptr)	// 1개 이상의 캐시된 청크가 있음, 청크단위 반환 상태
	{
		nextTail = cache.pop_front(); 
		releaseHead = nextTail->next;
		nextTail->next = nullptr;

		releaseSize = info.chunkCapacity;
	}
	else								// 1개 이상의 캐시된 청크가 없음, 즉 조각 반환 상태
	{									// 스레드 소멸시점에서남은걸 다 반환할때만 타는 분기
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
/// 모드에 따른 에러 체크
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
/// 유저 영역에서 사용하지 않는 메모리를 이용하여 메모리 오버플로우 체크.
/// true = error
/// </summary>
/// <returns>true = error</returns>
template <typename T>
bool TLSPool<T>::corruptedCheck(memoryBlock<T>* ptr)
{
	if (GET_MEMORYBLOCK_CORRUPTED_MASK(ptr->key) != CORRUPTED_CHECK_MASK)	// 미리 마스킹해둔 key 값이 변조되었나 확인한다.
		return true;
	
	return false;
}

/// <summary>
/// 반환된 메모리가 해당 풀(메인 풀)에서 나온 메모리가 맞는지 체크.
/// false = error
/// </summary>
/// <returns>false = error</returns>
template <typename T>
bool TLSPool<T>::poolCheck(memoryBlock<T>* ptr)
{
	if (GET_MEMORYBLOCK_POOL(ptr->poolPtr) != (intptr_t)info.poolPtr)		// 미리 저장해둔 mainPool의 값이 맞나 확인한다.
		return false;

	return true;
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 구현간 디버깅용 함수
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