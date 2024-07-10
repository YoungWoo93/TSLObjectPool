#include "../TLSObjectPool.h"

#include <iostream>
#include <vector>

using namespace std;


int main()
{
	cout << "test" << endl;
	vector<int*> v;

	for (int i = 0; i < 100; i++)
	{
		TLSObjectPool<int>::printTLSPool();
		auto temp = TLSObjectPool<int>::alloc();
		*temp = i;
		v.push_back(temp);
	}

	for (auto i : v)
	{
		TLSObjectPool<int>::free(i);
		TLSObjectPool<int>::printTLSPool();
	}

	v.clear();

	for (int i = 0; i < 100; i++)
	{
		auto temp = TLSObjectPool<int>::alloc();
		v.push_back(temp);
		TLSObjectPool<int>::printTLSPool();
	}

	for (auto i : v)
	{
		TLSObjectPool<int>::free(i);
		TLSObjectPool<int>::printTLSPool();
	}

	return 0;
}