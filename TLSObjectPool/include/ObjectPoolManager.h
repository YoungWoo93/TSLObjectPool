#pragma once

#include "define.h"
#include "MainPool.h"

#include <functional>
#include <mutex>
#include <vector>

using namespace std;

struct memoryPoolLogBlock {

};



class MemoryPoolManager {
public:
	static MemoryPoolManager& getInstance();
	MemoryPoolManager();
	~MemoryPoolManager();

	static void testFunction() {};

	// <기본 기능>
	void	registerMainPool(MainPoolParent* pool);		// main pool 추가등록

	void	update();
	void	frameWait();

	int		idleMemoryRelease();		// mainPool의 idleMemoryRelease 함수 호출 주체, 반환값 : 정리된 메모리 양
	int		idlePoolRelease();			// release threshold tick 이상으로 놀고있는 메인 풀 정리, 반환값 : 정리된 메모리 양
	// </기본 기능>



	/// <옵션 변경> ///
	unsigned long long int		getCheckTickInterval();			// mainPoolManager가 동작할 주기 설정
	unsigned long long int		setCheckTickInterval(unsigned long long int);		// 반환값 = 이전 주기

	unsigned long long int		getReleaseTickThreshold();			// 아무도 사용하지 않는 mainPool이 삭제되기 위해 필요한 시간
	unsigned long long int		setReleaseTickThreshold(unsigned long long int);		//		자원을 영원히 낭비 할 순 없지만, 곧 재사용 될 수도 있으므로
	/// </옵션 변경> ///



private:

public:


private:
	volatile unsigned long long int checkTick;
	volatile unsigned long long int releaseTick;

	vector<MainPoolParent*> poolList;
	std::mutex lock;
};