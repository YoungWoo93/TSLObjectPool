#pragma once

//#define _DEBUG_ALL_
//#define TLSOBJECTPOOL_DEBUG

#ifdef _DEBUG_ALL_

#ifndef CHUNKCACHE_DEBUG
#define CHUNKCACHE_DEBUG
#endif
#ifndef TLSPOOL_DEBUG
#define TLSPOOL_DEBUG
#endif
#ifndef TLSOBJECTPOOL_DEBUG
#define TLSOBJECTPOOL_DEBUG
#endif

#endif

#include "include/define.h"
#include "include/MainPool.h"
#include "include/TLSPool.h"
#include "include/ObjectPoolManager.h"

#include "src/TLSPool.hpp"
#include "src/chunkCache.hpp"
#include "src/MainPool.hpp"
#include "src/chunkStack.hpp"
#include "src/blockCollector.hpp"
#include "src/ObjectPoolManager.hpp"


template <typename T>
class TLSObjectPool
{
public:
	static void init(unsigned int maxChunks, unsigned int allocChunks)
	{
		if (objectPool == nullptr)
			objectPool = new TLSPool<T>(maxChunks, allocChunks);
	}

	static T* alloc()
	{
		if (objectPool == nullptr)
			objectPool = new TLSPool<T>();

		return objectPool->alloc();
	}

	static void free(T* var)
	{
		if (objectPool == nullptr)
			objectPool = new TLSPool<T>();

		objectPool->free(var);
	}

	static size_t usableSize()
	{
		return MainPool<T>::getInstance().usableSize();
	}

	static size_t usingSize()
	{
		return MainPool<T>::getInstance().usingSize();
	}

	static void release()
	{
		delete objectPool;
	}

#ifdef TLSOBJECTPOOL_DEBUG
	static void printTLSPool()
	{
		if (objectPool == nullptr)
			objectPool = new TLSPool<T>();

		objectPool->print();
	}
#endif //TLSOBJECTPOOL_DEBUG

private:
	TLSObjectPool() {};
	TLSObjectPool(const TLSObjectPool&) = delete;
	TLSObjectPool& operator=(const TLSObjectPool&) = delete;

private:
public:
	static __declspec(thread) TLSPool<T>* objectPool;

};

template <typename T>
__declspec(thread) TLSPool<T>* TLSObjectPool<T>::objectPool = nullptr;