#pragma once

#include "define.h"
#include "MainPool.h"

//#define CHUNKCACHE_DEBUG
//#define TLSPOOL_DEBUG

#ifdef _DEBUG_ALL_

#ifndef CHUNKCACHE_DEBUG
#define CHUNKCACHE_DEBUG
#endif
#ifndef TLSPOOL_DEBUG
#define TLSPOOL_DEBUG
#endif
#endif

template <typename BLOCKPTR>
class chunkCache
{
public:
	chunkCache(unsigned int maxChunkCount = 2);
	~chunkCache();

	void push_back(BLOCKPTR node);
	
	BLOCKPTR pop_front();
	BLOCKPTR pop_back();
	BLOCKPTR peek_front();
	BLOCKPTR peek_back();

#ifdef CHUNKCACHE_DEBUG
	void print();
#endif //CHUNKCACHE_DEBUG

private:
	inline BLOCKPTR* shiftFront(BLOCKPTR* cur);
	inline BLOCKPTR* shiftRear(BLOCKPTR* cur);


private:
	BLOCKPTR*		caches;
	BLOCKPTR*		head;
	BLOCKPTR*		tail;
	unsigned int	maxSize;
	unsigned int	curSize;
};


template <typename T>
class TLSPool
{
public :
	TLSPool(unsigned int maxChunks = 2, unsigned int allocChunks = 1);
	~TLSPool();

	T* alloc();
	void free(T*);
	
#ifdef TLSPOOL_DEBUG
	void print();
#endif //TLSPOOL_DEBUG

private:
	bool isError(memoryBlock<T>*);
	bool corruptedCheck(memoryBlock<T>*);
	bool poolCheck(memoryBlock<T>*);

	void allocateBlocks();
	void releaseBlocks();
	

public:
	MainPool<T>&					mainPool;

	poolInfo						info;
	size_t							size;

#ifdef TLSPOOL_DEBUG
	size_t							allocSize;
	size_t							totalAllocSize;
	size_t							totalFreeSize;
	size_t							allocChunkSize;
	size_t							freeChunkSize;
#endif //TLSPOOL_DEBUG

private:
	chunkCache<memoryBlock<T>*>		cache;
	long long int					offset;

	memoryBlock<T>*					head;
	memoryBlock<T>*					tail;

};
