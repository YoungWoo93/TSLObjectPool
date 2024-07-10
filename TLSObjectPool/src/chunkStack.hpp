#pragma once
#include <Windows.h>

#include "../include/define.h"
#include "../include/MainPool.h"


#define		MAKE_KEY(ptr, no)	((no << 48) | (intptr_t)ptr)
#define		MAKE_PTR(key)		(USER_MODE_MASK & (intptr_t)key)

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

template <typename T>
int chunkStack<T>::releaseChunk(unsigned int count)
{
	// 다른 조작 함수들은 기본적으로 포인터의 주소를 가지고 정지, 추가 탐색 조건을 설정했는데
	// 이곳만은 size를 가지고 한다. 불일치에서 오는 이질감이 불쾌하다.
	// 어쩔 수 없는 것 인가?
	int erasedCount = 0;

	AcquireSRWLockExclusive(&lock);
	{
		size_t remainSize = size;
		memoryChunk<T>* cur = top;

		while (remainSize > count + 1)
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
int chunkStack<T>::releaseChunk(memoryChunk<T>* target)
{
	int erasedCount = 0;

	AcquireSRWLockExclusive(&lock);
	{
		memoryChunk<T>* cur = top;
		if (cur == target)
		{
			decSize();
			top = cur->next;
			target->~memoryChunk<T>();
		}
		else
		{
			while (cur != nullptr) {
				if (cur->next == target) {
					decSize();
					erasedCount = 1;
					cur->next = target->next;
					target->~memoryChunk<T>();
					break;
				}
				cur = cur->next;
			}
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