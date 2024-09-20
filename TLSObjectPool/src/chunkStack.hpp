#pragma once
#include <Windows.h>

#include "../include/define.h"
#include "../include/MainPool.h"


#define		MAKE_KEY(ptr, no)	((no << 48) | (intptr_t)ptr)		// 락프리 스택의 ABA 문제 해결을 위한 key 매크로
#define		MAKE_PTR(key)		(USER_MODE_MASK & (intptr_t)key)

/// <summary>
/// 락프리 스택의 pop 동작.
/// 
/// chunkStack은 mainPool 내의 인스턴스이며, 멀티 스레드 환경에서 직접적인 경합이 발생하는 부분이다.
/// 동기화 객체 대신 락프리를 이용해 동기화 이슈를 대응한다.
/// 
/// sharedLock이 존재하는 이유는 define에 정의된 MEMORY_TIME_SCALE, MEMORY_SIZE_SCALE 을 지원하기 위함이다.
/// </summary>
template <typename T>
memoryChunk<T>* chunkStack<T>::pop()
{
	memoryChunk<T>* ret;

	AcquireSRWLockShared(&lock);
	{
		if (InterlockedDecrement64(&size) < 0)
		{
			ret = newChunk();
			InterlockedIncrement64(&size);
		}
		else
		{
			while (true)
			{
				ret = top;
				memoryChunk<T>* nextTop = ((memoryChunk<T>*)MAKE_PTR(ret))->next;

				if (InterlockedCompareExchangePointer((PVOID*)&top, nextTop, ret) == ret)
					break;
			}

			ret = (memoryChunk<T>*)MAKE_PTR(ret);
		}
	}
	ReleaseSRWLockShared(&lock);

	return ret;
}

/// <summary>
/// 락프리 스택의 push 동작.
/// 
/// chunkStack은 mainPool 내의 인스턴스이며, 멀티 스레드 환경에서 직접적인 경합이 발생하는 부분이다.
/// 동기화 객체 대신 락프리를 이용해 동기화 이슈를 대응한다.
/// 
/// sharedLock이 존재하는 이유는 define에 정의된 MEMORY_TIME_SCALE, MEMORY_SIZE_SCALE 을 지원하기 위함이다.
/// </summary>
template <typename T>
void chunkStack<T>::push(memoryChunk<T>* chunk)
{
	AcquireSRWLockShared(&lock);
	{
		chunk->lastUsedTick = GetTickCount64();

		intptr_t chunkKey = MAKE_KEY(chunk, pushNo);
		memoryChunk<T>* tempTop;

		while (true)
		{
			tempTop = top;
			chunk->next = tempTop;
			if (InterlockedCompareExchangePointer((PVOID*)&top, (void*)chunkKey, tempTop) == tempTop)
				break;
		}
		InterlockedIncrement64((LONG64*)(&size));
	}
	ReleaseSRWLockShared(&lock);
}

/// <summary>
/// 락프리 스택의 clear 동작.
/// 
/// OS로 자원을 회수하는 기능을 지원하기 위해 추가됨
/// </summary>
template <typename T>
int chunkStack<T>::clear() {
	memoryChunk<T>* nextTop;
	int ret = 0;

	while (top != nullptr)
	{
		nextTop = top->next;

		delete top;
		ret += sizeof(T);
		size--;
		top = nextTop;
	}

	return ret;
}

/// <summary>
/// 락프리 스택의 init 동작.
/// 
/// </summary>
template <typename T>
void chunkStack<T>::init(MCCAPACITY cap) 
{
	capacity = cap;
	size = 0; 
	top = nullptr; 
	lock = SRWLOCK_INIT; 
	pushNo = 0;
};

/// <summary>
/// define.h에 정의된 SCALE 동작을 지원하기 위한 함수.
/// </summary>
/// <returns> 삭제된 노드 (= 청크) 의 갯수</returns>
/*/
template <typename T>
int chunkStack<T>::releaseChunk(unsigned int count)
{
	// 다른 조작 함수들은 기본적으로 포인터의 주소를 가지고 정지, 추가 탐색 조건을 설정했는데
	// 이곳만은 size를 가지고 한다. 불일치에서 오는 이질감이 불쾌하다.
	// 어쩔 수 없는 것 인가?
	int erasedCount = 0;

	AcquireSRWLockExclusive(&lock);
	{
		long long int remainSize = size;
		memoryChunk<T>* cur = top;

		while (remainSize >= count)
		{
			cur = cur->next;
			remainSize--;
		}

		memoryChunk<T>* temp = cur;
		cur = cur->next;
		temp->next = nullptr;
		remainSize--;

		erasedCount = remainSize;
		decSize(remainSize);

		while (cur != nullptr) {
			memoryChunk<T>* next = cur->next;
			delete cur;
			cur = next;
		}
	}
	ReleaseSRWLockExclusive(&lock);

	return erasedCount;
}
/*/

/// <summary>
/// define.h에 정의된 SCALE 동작을 지원하기 위한 함수.
/// </summary>
/// <returns> 삭제된 노드 (= 청크) 의 갯수</returns>
template <typename T>
int chunkStack<T>::releaseChunk(unsigned long long int idleTime)
{
	int erasedCount = 0;
	unsigned long long int currentTick = GetTickCount64();
	auto timeDev = [](unsigned long long int curTic, unsigned long long int lastTic) {
		if (curTic > lastTic)
			return curTic - lastTic;

		return curTic + lastTic;
		};


	AcquireSRWLockExclusive(&lock);
	{
		memoryChunk<T>* cur = top;

		while (cur != nullptr) {
			memoryChunk<T>* target = cur->next;

			if (target != nullptr &&
				timeDev(currentTick, target->lastUsedTick) >= idleTime)
			{
				decSize();
				erasedCount++;

				cur->next = target->next;
				delete target;
			}
			cur = cur->next;
		}
	}
	ReleaseSRWLockExclusive(&lock);

	return erasedCount;
}

template <typename T>
memoryChunk<T>* chunkStack<T>::newChunk()
{
	return new memoryChunk<T>(capacity);
}

template <typename T>
void chunkStack<T>::decSize(int n)
{
	InterlockedAdd64(&size, n);
}
template <typename T>
void chunkStack<T>::incSize(int n)
{
	InterlockedAdd64(&size, -n);
}


#undef		MAKE_KEY
#undef		MAKE_PTR