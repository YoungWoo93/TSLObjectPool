#pragma once

#include "../include/TLSPool.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// chunkCache class
/// 
/// ûũ���� ��ȯ, �Ҵ��� �Ҷ� �տ������� ���� Ž���� �ϴ°��� �ƴ϶� ���κ��� ûũ ĳ�۽�Ƽ�� ��� ������ ������ ��带 ĳ���ϴ� �뵵.
/// 
/// �޸𸮰� ���� �����忡�� ���ǰ� �ǰ�, ���� OS���� �޸� ��ȯ���� ����ϸ� �ʿ������� �޸��� ����ȭ�� �Ͼ� �� �� �ۿ� ����
/// �� ����ȭ�� �ǿ����� ĳ�ø޸��� ȿ�� ���Ҹ� �غ��ϱ� ���� ������ ���̵��
///		-> ���� N���� ���� Ž�� �ϴٺ��� ���Ḯ��Ʈ ����� �̻� �ټ��� ĳ�ö��ο� ������ ��ĥ �� �ۿ� ���ٰ� �Ǵ�.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename BLOCKPTR>
chunkCache<BLOCKPTR>::chunkCache(unsigned int maxChunkCount)
{
	caches = new BLOCKPTR[maxChunkCount];
#ifdef _DEBUG
	memset(caches, 0, sizeof(BLOCKPTR) * maxChunkCount);
#endif //_DEBUG

	curSize = 0;
	maxSize = maxChunkCount;
	head = caches;
	tail = caches;
}

template <typename BLOCKPTR>
chunkCache<BLOCKPTR>::~chunkCache()
{
	delete caches;
}

/// <summary>
/// ���ο� �ּҸ� ����
/// </summary>
/// <param name="node"> ���� ��� (�ּ� ��)</param>
template <typename BLOCKPTR>
void chunkCache<BLOCKPTR>::push_back(BLOCKPTR node)
{
	*tail = node;
	tail = shiftRear(tail);
	curSize++;
}

/// <summary>
/// ���� �������� ���� �� �ּҸ� ����
/// </summary>
/// <returns> ���ŵ� �� (�ּ� ��), ������� �� nullptr </returns>
template <typename BLOCKPTR>
BLOCKPTR chunkCache<BLOCKPTR>::pop_front()
{
	if (curSize == 0)
		return (BLOCKPTR)nullptr;

	BLOCKPTR ret = *head;

#ifdef _DEBUG
	* head = 0;
#endif //_DEBUG

	head = shiftRear(head);
	curSize--;

	return ret;
}

/// <summary>
/// ���� �ֱٿ� ���� �� �ּҸ� ����
/// </summary>
/// <returns> ���ŵ� �� (�ּ� ��), ������� �� nullptr </returns>
template <typename BLOCKPTR>
BLOCKPTR chunkCache<BLOCKPTR>::pop_back()
{
	if (curSize == 0)
		return (BLOCKPTR)nullptr;

	tail = shiftFront(tail);
	BLOCKPTR ret = *tail;
	curSize--;


#ifdef _DEBUG
	*tail = 0;
#endif //_DEBUG

	return ret;
}

/// <summary>
/// ���� �������� ���� �� �ּҸ� Ȯ��
/// </summary>
/// <returns> Ȯ�ε� �� (�ּ� ��), ������� �� nullptr </returns>
template <typename BLOCKPTR>
BLOCKPTR chunkCache<BLOCKPTR>::Peek_front()
{
	if (curSize == 0)
		return nullptr;

	return *head;
}

/// <summary>
/// ���� �ֱٿ� ���� �� �ּҸ� Ȯ��
/// </summary>
/// <returns> Ȯ�ε� �� (�ּ� ��), ������� �� nullptr </returns>
template <typename BLOCKPTR>
BLOCKPTR chunkCache<BLOCKPTR>::Peek_back()
{
	if (curSize == 0)
		return nullptr;

	return *tail;
}


/// <summary>
/// head �Ǵ� tail�� �� ĭ�� shift ���ִ� ����. shift�� ����� �����Ϸ��� ��ȯ�� ���� ���� ���־�� �Ѵ�. 
/// </summary>
/// <param name="cur"> shift ����̵Ǵ� �ּ� ��, head �Ǵ� tail�� �� �� �ִ�. </param>
/// <returns> shift �� ������ �� (�ּ� ��) </returns>
template <typename BLOCKPTR>
inline BLOCKPTR* chunkCache<BLOCKPTR>::shiftFront(BLOCKPTR* cur)
{
	return caches + ((maxSize + cur - caches - 1) % maxSize);
}

/// <summary>
/// head �Ǵ� tail�� �� ĭ�� shift ���ִ� ����. shift�� ����� �����Ϸ��� ��ȯ�� ���� ���� ���־�� �Ѵ�. 
/// </summary>
/// <param name="cur"> shift ����̵Ǵ� �ּ� ��, head �Ǵ� tail�� �� �� �ִ�. </param>
/// <returns> shift �� ������ �� (�ּ� ��) </returns>
template <typename BLOCKPTR>
inline BLOCKPTR* chunkCache<BLOCKPTR>::shiftRear(BLOCKPTR* cur)
{
	return caches + ((maxSize + cur - caches + 1) % maxSize);
}






////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ������ ������ �Լ�
/// 
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#include <iostream>

template <typename BLOCKPTR>
void chunkCache<BLOCKPTR>::print()
{
	std::cout << "cache " << this << " << ";
	
	for (int i = 0; i < this->maxSize; i++)
	{
		std::cout << this->caches[i] << "\t";
	}

	std::cout << " (" << this->curSize << ")";

	std::cout << std::endl;
}
#endif //_DEBUG