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
	memoryBlock<T>* chunkTail;	//변수명 이게 최선인가
};





template <typename T>
class MainPool
{
public :
	static MainPool& getInstance();

	void releaseBlocks(memoryBlock<T>* head, memoryBlock<T>* tail, MCCAPACITY size);
	void newBlocks(memoryBlock<T>*& head, memoryBlock<T>*& tail);		/// 새로운 블록을 꺼낼때엔 항상 1. fillchunk를 받음, 2. fillchunk내의 메모리를 꺼냄, 3. emptychunk를 반환함, 4. 꺼낸 메모리를 할당해줌
								/// 왜냐하면 메모리를 먼저 줄경우 청크가 모자라는 일이 발생 할 수 있음
									/// => 진짜? 밥사와서 확인 ㄱㄱ
										/// 발생안함, TLS풀에 단1개의 블록이라도 존재하는 한, 모자랄 수는 없음
										/// TLS풀에서 조각 블록이 반환될때도, 블록 콜렉터에게 반환이 되므로 빈 청크가 모자라는 일은 없음

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