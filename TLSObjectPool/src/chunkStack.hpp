#pragma once
#include <Windows.h>

#include "../include/define.h"
#include "../include/MainPool.h"


#define		MAKE_KEY(ptr, no)	((no << 48) | (intptr_t)ptr)		// ������ ������ ABA ���� �ذ��� ���� key ��ũ��
#define		MAKE_PTR(key)		(USER_MODE_MASK & (intptr_t)key)

/// <summary>
/// ������ ������ pop ����.
/// 
/// chunkStack�� mainPool ���� �ν��Ͻ��̸�, ��Ƽ ������ ȯ�濡�� �������� ������ �߻��ϴ� �κ��̴�.
/// ����ȭ ��ü ��� �������� �̿��� ����ȭ �̽��� �����Ѵ�.
/// 
/// sharedLock�� �����ϴ� ������ define�� ���ǵ� MEMORY_TIME_SCALE, MEMORY_SIZE_SCALE �� �����ϱ� �����̴�.
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
/// ������ ������ push ����.
/// 
/// chunkStack�� mainPool ���� �ν��Ͻ��̸�, ��Ƽ ������ ȯ�濡�� �������� ������ �߻��ϴ� �κ��̴�.
/// ����ȭ ��ü ��� �������� �̿��� ����ȭ �̽��� �����Ѵ�.
/// 
/// sharedLock�� �����ϴ� ������ define�� ���ǵ� MEMORY_TIME_SCALE, MEMORY_SIZE_SCALE �� �����ϱ� �����̴�.
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
/// ������ ������ clear ����.
/// 
/// OS�� �ڿ��� ȸ���ϴ� ����� �����ϱ� ���� �߰���
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
/// ������ ������ init ����.
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
/// define.h�� ���ǵ� SCALE ������ �����ϱ� ���� �Լ�.
/// </summary>
/// <returns> ������ ��� (= ûũ) �� ����</returns>
/*/
template <typename T>
int chunkStack<T>::releaseChunk(unsigned int count)
{
	// �ٸ� ���� �Լ����� �⺻������ �������� �ּҸ� ������ ����, �߰� Ž�� ������ �����ߴµ�
	// �̰����� size�� ������ �Ѵ�. ����ġ���� ���� �������� �����ϴ�.
	// ��¿ �� ���� �� �ΰ�?
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
/// define.h�� ���ǵ� SCALE ������ �����ϱ� ���� �Լ�.
/// </summary>
/// <returns> ������ ��� (= ûũ) �� ����</returns>
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