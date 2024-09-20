#pragma once

#include "../include/ObjectPoolManager.h"


using namespace std;

MemoryPoolManager& MemoryPoolManager::getInstance()
{
	static MemoryPoolManager instance;

	return instance;
}

MemoryPoolManager::MemoryPoolManager()
{
	checkTick = 1 * 60 * 1000;
	releaseTick = 5 * 60 * 1000;
}

MemoryPoolManager::~MemoryPoolManager()
{

}

// <기본 기능>
void MemoryPoolManager::registerMainPool(MainPoolParent* pool)		// main pool 추가등록
{
	lock.lock();

	for (int i = 0; i < poolList.size(); i++)
	{
		if (poolList[i] == pool)
		{
			lock.unlock();
			return;
		}
	}

	poolList.push_back(pool);
	lock.unlock();
}


void MemoryPoolManager::update()
{
	while (true)
	{
		frameWait();
		idleMemoryRelease();
		idlePoolRelease();
	}
}
void MemoryPoolManager::frameWait()
{
	static long long int lastTick =GetTickCount64();

	lastTick += checkTick;
	long long int sleepTick = lastTick - GetTickCount64();

	Sleep(sleepTick);
}

int	MemoryPoolManager::idleMemoryRelease()
{
	int ret = 0;
	lock.lock();

	for (int i = 0; i < poolList.size(); i++)
	{
		if (poolList[i]->referenceCount > 0)
			ret += poolList[i]->idleMemoryRelease();
	}

	lock.unlock();
	return ret;
}
int	MemoryPoolManager::idlePoolRelease()
{
	int ret = 0;
	unsigned long long int currentTick = GetTickCount64();

	lock.lock();

	for (int i = 0; i < poolList.size(); i++)
	{
		if (poolList[i]->referenceCount < 1 && poolList[i]->releaseTick + releaseTick < currentTick)
			poolList[i]->clear();
	}

	lock.unlock();
	return 0;
}


unsigned long long int MemoryPoolManager::getCheckTickInterval()
{
	return checkTick;
}
unsigned long long int MemoryPoolManager::setCheckTickInterval(unsigned long long int tick)
{
	return InterlockedExchange64((long long int*)(&checkTick), tick);
}

unsigned long long int MemoryPoolManager::getReleaseTickThreshold()	
{
	return releaseTick;
}
unsigned long long int MemoryPoolManager::setReleaseTickThreshold(unsigned long long int tick)
{
	return InterlockedExchange64((long long int*)(&releaseTick), tick);
}
