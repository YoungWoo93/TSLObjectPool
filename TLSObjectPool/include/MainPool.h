#pragma once

#include <Windows.h>

#include "define.h"

template <typename T>
class chunkStack
{
public :
	chunkStack(MCCAPACITY cap) { capacity = cap; size = 0; top = nullptr; lock = SRWLOCK_INIT; pushNo = 0; };
	~chunkStack() {};

	memoryChunk<T>* pop();
	void push(memoryChunk<T>* chunk);

	int releaseChunk(unsigned int count);
	int releaseChunk(unsigned long long int idleTime);
	int releaseChunk(memoryChunk<T>* target);

private:
	memoryChunk<T>* newChunk();
	void decSize(int n = 1);
	void incSize(int n = 1);

public:
	MCCAPACITY				capacity;
	volatile long long int	size;

private:
	memoryChunk<T>*	top;
	SRWLOCK			lock;
	intptr_t		pushNo;
};





template <typename T>
class blockCollector
{
public :
	blockCollector() {};
	~blockCollector() {};

	bool collect(memoryBlock<T>* _head, memoryBlock<T>* _tail, size_t size) { return true; };
	void defragment(memoryChunk<T>* chunk) {};


public:
	size_t			size;
	MCCAPACITY		chunkCapacity;

private:
	memoryBlock<T>*	head;
	memoryBlock<T>* tail;
	memoryBlock<T>* chunkTail;	//������ �̰� �ּ��ΰ�
};





template <typename T>
class MainPool
{
public :
	static MainPool& getInstance();

	void releaseBlocks(memoryBlock<T>* head, memoryBlock<T>* tail, MCCAPACITY size);
	void newBlocks(memoryBlock<T>*& head, memoryBlock<T>*& tail);		/// ���ο� ����� �������� �׻� 1. fillchunk�� ����, 2. fillchunk���� �޸𸮸� ����, 3. emptychunk�� ��ȯ��, 4. ���� �޸𸮸� �Ҵ�����
								/// �ֳ��ϸ� �޸𸮸� ���� �ٰ�� ûũ�� ���ڶ�� ���� �߻� �� �� ����
									/// => ��¥? ���ͼ� Ȯ�� ����
										/// �߻�����, TLSǮ�� ��1���� ����̶� �����ϴ� ��, ���ڶ� ���� ����
										/// TLSǮ���� ���� ����� ��ȯ�ɶ���, ��� �ݷ��Ϳ��� ��ȯ�� �ǹǷ� �� ûũ�� ���ڶ�� ���� ����

	size_t usableSize();
	static size_t usingSize();

private:
	MainPool(MCCAPACITY chunkCapacity = DEFAULT_CHUNK_CAPACITY, MPOPTION mode = 0);
	~MainPool();

	


public:
	poolInfo info;

private:
	chunkStack<T>		emptyChunks;
	chunkStack<T>		fillChunks;
	blockCollector<T>	collector;
};