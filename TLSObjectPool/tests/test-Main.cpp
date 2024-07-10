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
	auto t = TLSObjectPool<int>::alloc();
	TLSObjectPool<double>::free((double*)t);

	return 0;
}


int main()
{
	test2();
	return 0;
	for (int i = 0; i < 1000000; i++)
	{
		std::thread testThread(test1);

		testThread.join();

		for (auto g : garbage)
		{
			TLSObjectPool<int>::free(g);
		}
		garbage.clear();

		if (i % 100000 == 0)
		{
			cout << i / 10000 << "%\t" << TLSObjectPool<int>::usingSize() << "\t/\t" << TLSObjectPool<int>::usableSize() << endl;
		}
	}

	cout << TLSObjectPool<int>::usingSize() << "\t/\t" << TLSObjectPool<int>::usableSize() << endl;
	
	return 0;
}