#pragma once

#include "define.h"
#include "MainPool.h"


template <typename BLOCKPTR>
class chunkCache
{
public:
	chunkCache(unsigned int maxChunkCount = 1);
	~chunkCache();

	void push_back(BLOCKPTR node);
	
	BLOCKPTR pop_front();
	BLOCKPTR pop_back();
	BLOCKPTR Peek_front();
	BLOCKPTR Peek_back();

#ifdef _DEBUG
	void print();
#endif //_DEBUG

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
	
#ifdef _DEBUG
	void print();
#endif //_DEBUG

private:
	bool isError(memoryBlock<T>*);
	bool flowCheck(memoryBlock<T>*);
	bool poolCheck(memoryBlock<T>*);

	void allocateBlocks();
	void releaseBlocks();
	void releaseBlocks(memoryBlock<T>* _head, memoryBlock<T>* _tail, size_t _size);
	

public:
	MainPool<T>&					mainPool;

	poolInfo						info;	//8경계, 24바이트
	size_t							size;

#ifdef _DEBUG
	size_t							allocSize;
	size_t							totalAllocSize;
	size_t							totalFreeSize;
	size_t							allocChunkSize;
	size_t							freeChunkSize;
#endif

private:
	chunkCache<memoryBlock<T>*>		cache;	//8경계, 32바이트
	long long int					offset;

	memoryBlock<T>*					head;
	memoryBlock<T>*					tail;

};
