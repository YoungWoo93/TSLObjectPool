#pragma once

#include <Windows.h>

#include "define.h"

template <typename T>
class chunkStack
{
public :
	chunkStack(MCCAPACITY cap) { 
		capacity = cap; size = 0; top = nullptr; lock = SRWLOCK_INIT; pushNo = 0; };
	~chunkStack() {
		memoryChunk<T>* nextTop;

		while (top != nullptr)
		{
			nextTop = top->next;

			delete top;
			size--;
			top = nextTop;
		}
	};

	memoryChunk<T>* pop();
	void push(memoryChunk<T>* chunk);

	//int releaseChunk(unsigned int count);				// [todo] ��ĥ �ڿ� �ٽ� ���� �� �Լ��� �� ��Ȳ�� �ִ��� ����غ���
														// ���� �������δ� ����� ���Ǵ� �޸𸮸� ������ �����ص� ������ �ٽ� �Ҵ� �� �� ����.
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
class MainPool
{
public:
	static MainPool& getInstance();

	void releaseBlocks(memoryBlock<T>* head, memoryBlock<T>* tail, MCCAPACITY size);
	void newBlocks(memoryBlock<T>*& head, memoryBlock<T>*& tail);

	size_t usableSize();
	size_t usingSize();

private:
	MainPool(MCCAPACITY chunkCapacity = DEFAULT_CHUNK_CAPACITY, MPOPTION mode = 0);
	~MainPool();


	//[todo] ��ĥ �ڿ� �ٽ� ���� �� Ŭ������ �� ������ Ŭ������ �����ؾ� �ϴ��� ����غ���
	// ���� �������δ� mainPool�� �ܼ��� chunk�� stackó�� ����ϴ� ������ �������̽� �̱� �ٶ���.
	// �׷��� ������ ����� ���忡�� �ܼ� stack�� ���� �̻��� �͵��� ���� Ŭ������ �и��ϴ°� �� ������ ����.
	class blockCollector
	{
	public:
		blockCollector(MCCAPACITY capcacity, MainPool<T>* mainPool);
		~blockCollector();

		bool collect(memoryBlock<T>* _head, memoryBlock<T>* _tail, MCCAPACITY _size);

	public:
		size_t			size;
		MCCAPACITY		chunkCapacity;

	private:
		memoryBlock<T>*	head;
		memoryBlock<T>*	tail;
		memoryBlock<T>*	chunkTail;
		MainPool<T>*	pool;
		SRWLOCK			lock;
	};

public:
	poolInfo info;

private:
	chunkStack<T>		emptyChunks;
	chunkStack<T>		fillChunks;
	blockCollector		collector;

};