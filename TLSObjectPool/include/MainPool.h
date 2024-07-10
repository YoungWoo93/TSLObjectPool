#pragma once

#include <Windows.h>

#include "define.h"

template <typename T>
class chunkStack
{
public :
	chunkStack(MCCAPACITY cap) { capacity = cap; size = 0; top = nullptr; lock = SRWLOCK_INIT; pushNo = 0; };
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


/*/
template <typename T>
class MainPool;

template <typename T>
class blockCollector
{
public :
	blockCollector(MCCAPACITY capcacity, MainPool<T>* mainPool);
	~blockCollector();

	bool collect(memoryBlock<T>* _head, memoryBlock<T>* _tail, MCCAPACITY _size);
	void defragment(memoryChunk<T>* chunk);


public:
	size_t			size;
	MCCAPACITY		chunkCapacity;
	
private:
	memoryBlock<T>*	head;
	memoryBlock<T>* tail;
	MainPool<T>* pool;

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
/*/



template <typename T>
class MainPool
{
public:
	static MainPool& getInstance();

	void releaseBlocks(memoryBlock<T>* head, memoryBlock<T>* tail, MCCAPACITY size);
	void newBlocks(memoryBlock<T>*& head, memoryBlock<T>*& tail);		/// ���ο� ����� �������� �׻� 1. fillchunk�� ����, 2. fillchunk���� �޸𸮸� ����, 3. emptychunk�� ��ȯ��, 4. ���� �޸𸮸� �Ҵ�����
	/// �ֳ��ϸ� �޸𸮸� ���� �ٰ�� ûũ�� ���ڶ�� ���� �߻� �� �� ����
		/// => ��¥? ���ͼ� Ȯ�� ����
			/// �߻�����, TLSǮ�� ��1���� ����̶� �����ϴ� ��, ���ڶ� ���� ����
			/// TLSǮ���� ���� ����� ��ȯ�ɶ���, ��� �ݷ��Ϳ��� ��ȯ�� �ǹǷ� �� ûũ�� ���ڶ�� ���� ����

	size_t usableSize();
	size_t usingSize();

private:
	MainPool(MCCAPACITY chunkCapacity = DEFAULT_CHUNK_CAPACITY, MPOPTION mode = 0);
	~MainPool();



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
		memoryBlock<T>* head;
		memoryBlock<T>* tail;
		memoryBlock<T>* chunkTail;	//������ �̰� �ּ��ΰ�
		MainPool<T>* pool;
	};

public:
	poolInfo info;

private:
	chunkStack<T>		emptyChunks;
	chunkStack<T>		fillChunks;
	blockCollector		collector;

};