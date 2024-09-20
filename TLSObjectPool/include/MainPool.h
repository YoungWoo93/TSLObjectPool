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

	void init(MCCAPACITY cap);
	int clear();

	//int releaseChunk(unsigned int count);				// [todo] 며칠 뒤에 다시 보고 이 함수가 쓸 상황이 있는지 고민해보기
														// 지금 생각으로는 빈번히 사용되는 메모리를 무조건 해제해도 조만간 다시 할당 될 것 같다.
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

class MainPoolParent {
public:

	MainPoolParent() {};
	virtual ~MainPoolParent() {};
	virtual int clear() { return 0; };
	virtual int idleMemoryRelease() { return 0; };

	UINT64				releaseTick;
	volatile LONG		referenceCount;
};

template <typename T>
class MainPool : public MainPoolParent
{
public:
	static MainPool& getInstance();

	void releaseBlocks(memoryBlock<T>* head, memoryBlock<T>* tail, MCCAPACITY size);
	void newBlocks(memoryBlock<T>*& head, memoryBlock<T>*& tail);
	int idleMemoryRelease() { return 0; };

	size_t usableSize();
	size_t usingSize();

private:
	MainPool(MCCAPACITY chunkCapacity = DEFAULT_CHUNK_CAPACITY, MPOPTION mode = 0); // 여기 초기 생성자 값을 왜 넣고있지?
	~MainPool() override;

	void init(MCCAPACITY chunkCapacity, MPOPTION mode);
	int clear();

	class blockCollector
	{
	public:
		blockCollector(MCCAPACITY capcacity, MainPool<T>* mainPool);
		~blockCollector();

		bool collect(memoryBlock<T>* _head, memoryBlock<T>* _tail, MCCAPACITY _size);

		void init(MCCAPACITY capacity, MainPool<T>* mainPool);
		int clear();

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
	poolInfo			info;
	blockCollector		collector;

	
private:
	chunkStack<T>		emptyChunks;
	chunkStack<T>		fillChunks;
	SRWLOCK				lock;
};