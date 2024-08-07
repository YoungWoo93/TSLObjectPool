#pragma once

#include "../include/TLSPool.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// chunkCache class
/// 
/// 청크단위 반환, 할당을 할때 앞에서부터 선형 탐색을 하는것이 아니라 헤드로부터 청크 캐퍼시티의 배수 단위로 떨어진 노드를 캐싱하는 용도.
///		-> 전체 프로세스 시간중 메모리 블록을 N개 선형탐색 하는 시간은 별 것 아니겠지만, 캐싱이 가능하다면 더 좋을 것이라 판단.
/// 
/// 메모리가 여러 스레드에서 사용되게 되고, 이후 OS로의 메모리 반환까지 고려하면 필연적으로 메모리의 파편화가 일어 날 수 밖에 없음
/// 이 파편화의 악영향중 캐시메모리의 효율 감소를 극복하기 위해 도입한 아이디어
///		-> 직접 N개를 선형 탐색 하다보면 연결리스트 방식인 이상 다수의 캐시라인에 영향을 미칠 수 밖에 없다고 판단.
/// 
/// TLSPool은 일정 숫자 이상의 메모리 블록을 소유하게 되면 메모리를 mainPool로 반환하기 때문에 소유할 수 있는 최대 메모리 블록의 수가 정해져있음
/// 그렇기 때문에 정해진 크기의 배열에 해제를 위한 메모리 주소를 저장해 두는 형태 (환형 큐 형태)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



template <typename BLOCKPTR>
chunkCache<BLOCKPTR>::chunkCache(unsigned int maxChunkCount)
{
	caches = new BLOCKPTR[maxChunkCount];
#ifdef CHUNKCACHE_DEBUG
	memset(caches, 0, sizeof(BLOCKPTR) * maxChunkCount);
#endif //CHUNKCACHE_DEBUG

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
/// 새로운 주소를 삽입
/// </summary>
/// <param name="node"> 삽입 대상 (주소 값)</param>
template <typename BLOCKPTR>
void chunkCache<BLOCKPTR>::push_back(BLOCKPTR node)
{
	*tail = node;
	tail = shiftRear(tail);
	curSize++;
}

/// <summary>
/// 가장 오래전에 삽입 된 주소를 제거
/// </summary>
/// <returns> 제거된 값 (주소 값), 비어있을 시 nullptr </returns>
template <typename BLOCKPTR>
BLOCKPTR chunkCache<BLOCKPTR>::pop_front()
{
	if (curSize == 0)
		return (BLOCKPTR)nullptr;

	BLOCKPTR ret = *head;

#ifdef CHUNKCACHE_DEBUG
	* head = 0;
#endif //CHUNKCACHE_DEBUG

	head = shiftRear(head);
	curSize--;

	return ret;
}

/// <summary>
/// 가장 최근에 삽입 된 주소를 제거
/// </summary>
/// <returns> 제거된 값 (주소 값), 비어있을 시 nullptr </returns>
template <typename BLOCKPTR>
BLOCKPTR chunkCache<BLOCKPTR>::pop_back()
{
	if (curSize == 0)
		return (BLOCKPTR)nullptr;

	tail = shiftFront(tail);
	BLOCKPTR ret = *tail;
	curSize--;


#ifdef CHUNKCACHE_DEBUG
	*tail = 0;
#endif //CHUNKCACHE_DEBUG

	return ret;
}

/// <summary>
/// 가장 오래전에 삽입 된 주소를 확인
/// </summary>
/// <returns> 확인된 값 (주소 값), 비어있을 시 nullptr </returns>
template <typename BLOCKPTR>
BLOCKPTR chunkCache<BLOCKPTR>::peek_front()
{
	if (curSize == 0)
		return nullptr;

	return *head;
}

/// <summary>
/// 가장 최근에 삽입 된 주소를 확인
/// </summary>
/// <returns> 확인된 값 (주소 값), 비어있을 시 nullptr </returns>
template <typename BLOCKPTR>
BLOCKPTR chunkCache<BLOCKPTR>::peek_back()
{
	if (curSize == 0)
		return nullptr;

	return *tail;
}


/// <summary>
/// head 또는 tail을 한 칸씩 shift 해주는 동작. shift한 결과를 갱신하려면 반환된 값을 대입 해주어야 한다. 
/// </summary>
/// <param name="cur"> shift 대상이되는 주소 값, head 또는 tail이 올 수 있다. </param>
/// <returns> shift 된 이후의 값 (주소 값) </returns>
template <typename BLOCKPTR>
inline BLOCKPTR* chunkCache<BLOCKPTR>::shiftFront(BLOCKPTR* cur)
{
	return caches + ((maxSize + cur - caches - 1) % maxSize);
}

/// <summary>
/// head 또는 tail을 한 칸씩 shift 해주는 동작. shift한 결과를 갱신하려면 반환된 값을 대입 해주어야 한다. 
/// </summary>
/// <param name="cur"> shift 대상이되는 주소 값, head 또는 tail이 올 수 있다. </param>
/// <returns> shift 된 이후의 값 (주소 값) </returns>
template <typename BLOCKPTR>
inline BLOCKPTR* chunkCache<BLOCKPTR>::shiftRear(BLOCKPTR* cur)
{
	return caches + ((maxSize + cur - caches + 1) % maxSize);
}






////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 구현간 디버깅용 함수
/// 
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef CHUNKCACHE_DEBUG
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
#endif //CHUNKCACHE_DEBUG