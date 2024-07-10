#pragma once
#include "../include/MainPool.h"


template <typename T>
MainPool<T>& MainPool<T>::getInstance()
{
	static MainPool<T> instance;

	return instance;
}
template <typename T>
void MainPool<T>::releaseBlocks(memoryBlock<T>* head, memoryBlock<T>* tail, MCCAPACITY size)
{
	if (size != info.chunkCapacity)
	{
		collector.collect(head, tail, size);
		return;
	}

	memoryChunk<T>* chunk = emptyChunks.pop();
	chunk = new (chunk) memoryChunk<T>(head, tail, size);

	fillChunks.push(chunk);
}
template <typename T>
void MainPool<T>::newBlocks(memoryBlock<T>*& head, memoryBlock<T>*& tail)
{
	memoryChunk<T>* chunk = fillChunks.pop();

	head = (memoryBlock<T>*)(chunk->blockStart);
	tail = (memoryBlock<T>*)(chunk->blockEnd);
	
	chunk->blockStart = nullptr;
	chunk->blockEnd = nullptr;

	emptyChunks.push(chunk);
}

template <typename T>
size_t MainPool<T>::usableSize()
{
	return fillChunks.size * fillChunks.capacity;
}

template <typename T>
size_t MainPool<T>::usingSize()
{
	return emptyChunks.size * emptyChunks.capacity;
}



template <typename T>
MainPool<T>::MainPool(MCCAPACITY chunkCapacity, MPOPTION mode) 
	: emptyChunks(chunkCapacity), fillChunks(chunkCapacity), collector(chunkCapacity, this)
{
	info.chunkCapacity = chunkCapacity;
	info.mode = mode;
	info.poolPtr = this;
}
template <typename T>
MainPool<T>::~MainPool()
{

	// 메인풀의 소멸자 호출시점 = 프로세스 종료시점 이므로 생략
}
