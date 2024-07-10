#include "../TLSObjectPool.h"

#include <iostream>
#include <thread>
#include <vector>

using namespace std;


vector<int*> garbage;
int test1()
{
	//cout << "test" << endl;
	vector<int*> v;

	for (int i = 0; i < 100; i++)
	{
		//TLSObjectPool<int>::printTLSPool();
		auto temp = TLSObjectPool<int>::alloc();
		*temp = i;
		v.push_back(temp);
	}

	for (auto i : v)
	{
		TLSObjectPool<int>::free(i);
		//TLSObjectPool<int>::printTLSPool();
	}

	v.clear();

	for (int i = 0; i < 100; i++)
	{
		auto temp = TLSObjectPool<int>::alloc();
		v.push_back(temp);
		//TLSObjectPool<int>::printTLSPool();
	}

	garbage.push_back(TLSObjectPool<int>::alloc());
	garbage.push_back(TLSObjectPool<int>::alloc());

	for (auto i : v)
	{
		TLSObjectPool<int>::free(i);
		//TLSObjectPool<int>::printTLSPool();
	}

	TLSObjectPool<int>::release();
	return 0;
}


int test2()
{
	int testCycle = 500000;
	for (int i = 0; i < testCycle; i++)
	{
		std::thread testThread(test1);

		testThread.join();

		for (auto g : garbage)
		{
			TLSObjectPool<int>::free(g);
		}
		garbage.clear();

		if (i % (testCycle / 100) == 0)
		{
			cout << i / (testCycle / 100) << "%\t" << TLSObjectPool<int>::usingSize() << "\t/\t" << TLSObjectPool<int>::usableSize() << endl;
		}
	}

	cout << TLSObjectPool<int>::usingSize() << "\t/\t" << TLSObjectPool<int>::usableSize() << endl;

	return 0;
}



int test3()
{
	auto t = TLSObjectPool<int>::alloc();
	TLSObjectPool<double>::free((double*)t);

	return 0;
}


int main()
{
	test3();
	
	return 0;
}