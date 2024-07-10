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
	/// �⺻ ������, empty chunk ����.
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
	/// size��ŭ�� �޸� ����� �������ִ� ûũ ���� (fill chunk ����)
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
	/// �� ������ �޸� ����� ���Ḯ��Ʈ�� ��� ûũ �ʱ�ȭ.
	/// placement new ��������.
	/// </summary>
	/// <param name="start"> ���� �޸𸮺�� </param>
	/// <param name="end"> ���� �޸𸮺�� </param>
	/// <param name="size"> �޸� ����� ���� </param>
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
	/// ûũ ������ ������ �޸� ��϶��� �����ǰ� �ϱ����� �Ҹ���.
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

	unsigned short	allocChunckUnit;			//�ʱ� �Ҵ�� ��� ûũ�� �޾ƿ���
	unsigned long long int	blockThreshold;		//� �̻��� ����� �׿��� 1ûũ�� ��ȯ�ϴ°�
};

