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

	// <�⺻ ���>
	void	registerMainPool(MainPoolParent* pool);		// main pool �߰����

	void	update();
	void	frameWait();

	int		idleMemoryRelease();		// mainPool�� idleMemoryRelease �Լ� ȣ�� ��ü, ��ȯ�� : ������ �޸� ��
	int		idlePoolRelease();			// release threshold tick �̻����� ����ִ� ���� Ǯ ����, ��ȯ�� : ������ �޸� ��
	// </�⺻ ���>



	/// <�ɼ� ����> ///
	unsigned long long int		getCheckTickInterval();			// mainPoolManager�� ������ �ֱ� ����
	unsigned long long int		setCheckTickInterval(unsigned long long int);		// ��ȯ�� = ���� �ֱ�

	unsigned long long int		getReleaseTickThreshold();			// �ƹ��� ������� �ʴ� mainPool�� �����Ǳ� ���� �ʿ��� �ð�
	unsigned long long int		setReleaseTickThreshold(unsigned long long int);		//		�ڿ��� ������ ���� �� �� ������, �� ���� �� ���� �����Ƿ�
	/// </�ɼ� ����> ///



private:

public:


private:
	volatile unsigned long long int checkTick;
	volatile unsigned long long int releaseTick;

	vector<MainPoolParent*> poolList;
	std::mutex lock;
};