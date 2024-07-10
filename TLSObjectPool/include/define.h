#pragma once

#include <Windows.h>

#define	DEFAULT_CHUNK_CAPACITY	1000

#define DEBUG					1
#define MEMORY_CORRUPTED_CHECK	2
#define MEMORY_TIME_SCALE		4
#define MEMORY_SIZE_SCALE		8

#define OPTION_5				16
#define OPTION_6				32
#define OPTION_7				64
#define OPTION_8				128

#define KERNEL_MODE_MASK		0xFFFF800000000000
#define USER_MODE_MASK			(~KERNEL_MODE_MASK) //0x00007FFFFFFFFFFF
#define CORRUPTED_CHECK_MASK	0xAAAA800000000000

#define	ENCODEING_MEMORYBLOCK_KEY(POOL)		((intptr_t)POOL | CORRUPTED_CHECK_MASK)
#define GET_MEMORYBLOCK_POOL(KEY)			((intptr_t)KEY & USER_MODE_MASK)		
#define GET_MEMORYBLOCK_CORRUPTED_MASK(KEY)	((intptr_t)KEY & CORRUPTED_CHECK_MASK)		

typedef unsigned int		MCCAPACITY;
typedef unsigned short		MPOPTION;

template <typename T>
struct memoryBlock {
	memoryBlock() { key = 0; }
	memoryBlock(void* pool) { poolPtr = pool; }

	~memoryBlock() {}
	T value;
	union {
		intptr_t key;
		void* poolPtr;
		memoryBlock<T>* next;
	};
};


template <typename T>
struct memoryChunk
{
	/// <summary>
	/// 기본 생성자, empty chunk 생성.
	/// </summary>
	memoryChunk()
	{
		next = nullptr;
		blockStart = nullptr;
		blockEnd = nullptr;
		capacity = 0;
		lastUsedTick = GetTickCount64();
	}

	/// <summary>
	/// size만큼의 메모리 블록을 가지고있는 청크 생성 (fill chunk 생성)
	/// </summary>
	/// <param name="size"></param>
	memoryChunk(MCCAPACITY size)
	{
		next = nullptr;
		blockStart = new memoryBlock<T>();

		memoryBlock<T>* temp = blockStart;
		for (MCCAPACITY i = 1; i < size - 1; i++)
		{
			temp->next = new memoryBlock<T>();
			temp = temp->next;
		}
		temp->next = new memoryBlock<T>();

		blockEnd = temp->next;
		blockEnd->next = nullptr;
		capacity = size;
		lastUsedTick = GetTickCount64();
	}

	/// <summary>
	/// 기 생성된 메모리 블록의 연결리스트를 담는 청크 초기화.
	/// placement new 지원목적.
	/// </summary>
	/// <param name="start"> 시작 메모리블록 </param>
	/// <param name="end"> 종료 메모리블록 </param>
	/// <param name="size"> 메모리 블록의 갯수 </param>
	memoryChunk(memoryBlock<T>* start, memoryBlock<T>* end, MCCAPACITY size)
	{
		next = nullptr;
		blockStart = start;
		blockEnd = end;
		blockEnd->next = nullptr;
		capacity = size;
		lastUsedTick = GetTickCount64();
	}

	/// <summary>
	/// 청크 삭제시 내부의 메모리 블록또한 삭제되게 하기위한 소멸자.
	/// </summary>
	~memoryChunk()
	{
		memoryBlock<T>* nextHead;

		while (blockStart != nullptr)
		{
			nextHead = blockStart->next;
			delete blockStart;
			blockStart = nextHead;
		}
	}

	memoryChunk* next;
	memoryBlock<T>* blockStart;
	memoryBlock<T>* blockEnd;

	MCCAPACITY capacity;
	unsigned long long int lastUsedTick;
};



struct poolInfo {
	void* poolPtr;

	MCCAPACITY	chunkCapacity;
	MPOPTION	mode;

	unsigned short	allocChunckUnit;			//초기 할당시 몇개의 청크를 받아오나
	unsigned long long int	blockThreshold;		//몇개 이상의 블록이 쌓여야 1청크씩 반환하는가
};

