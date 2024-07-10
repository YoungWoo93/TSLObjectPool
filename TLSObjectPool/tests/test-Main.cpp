#include "../TLSObjectPool.h"

#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <thread>
#include <vector>

using namespace std;

thread_local std::mt19937 rng(std::random_device{}());



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


template <typename T>
class test4_IPC_PIPE
{
public:
	test4_IPC_PIPE(int threadSize)
	{
		size = threadSize;
		pipe.assign(size, vector<vector<T*>>(size, vector<T*>()));
		locks.assign(size, vector<SRWLOCK>(size, SRWLOCK_INIT));
	}

	bool push(int myIndex, int targetIndex, T* value)
	{
		bool ret = false;

		AcquireSRWLockExclusive(&locks[targetIndex][myIndex]);
		if (pipe[targetIndex][myIndex].size() <= 1000)
		{
			pipe[targetIndex][myIndex].push_back(value);
			ret = true;
		}
		ReleaseSRWLockExclusive(&locks[targetIndex][myIndex]);

		return ret;
	}

	void pop(int index, vector<T*>& container)
	{
		for (int i = 0; i < size; i++)
		{
			AcquireSRWLockExclusive(&locks[index][i]);
			for (auto v : pipe[index][i])
			{
				container.push_back(v);
			}
			pipe[index][i].clear();

			ReleaseSRWLockExclusive(&locks[index][i]);
		}
		
	}
public:
	int size;
	vector<vector<vector<T*>>> pipe;
	vector<vector<SRWLOCK>> locks;
};

struct test4_2k_struct {
	test4_2k_struct() {};
	~test4_2k_struct() {};
	char temp[2048];
};

struct test4_17k_struct {
	test4_17k_struct() {};
	~test4_17k_struct() {};
	char temp[17408];
};

struct test4_64k_struct {
	test4_64k_struct() {};
	~test4_64k_struct() {};
	char temp[65536];
};

//#define TEST4TYPE	int
//#define TEST4TYPE	test4_2k_struct
#define TEST4TYPE	test4_17k_struct
//#define TEST4TYPE	test4_64k_struct

int test4_thread_size = 10;
int test4Mode = 1;
test4_IPC_PIPE<TEST4TYPE> p(test4_thread_size);

template <typename T>
T* test4_allocFunc()
{
	switch (test4Mode)
	{
	case 1:
		return TLSObjectPool<T>::alloc();
		break;
	case 2:
		return new T;
		break;
	case 3:
		break;
	}
}

template <typename T>
void test4_freeFunc(T* v)
{
	switch (test4Mode)
	{
	case 1:
		TLSObjectPool<T>::free(v);
		break;
	case 2:
		delete v;
		break;
	case 3:
		break;
	}

}

template <typename T>
int test4_func(int index)
{
	TLSObjectPool<T>::init(3, 2);
	std::uniform_int_distribution<int> dist(0, 30);
	std::uniform_int_distribution<int> dist2(0, 30);
	std::uniform_int_distribution<int> dist3(0, test4_thread_size - 1);
	vector<T*> temp;
	int myUse = dist(rng);

	for (int i = 0; i < 100000; i++)
	{
		if (dist2(rng) > myUse)
		{
			p.pop(index, temp);

			for (auto v : temp)
			{
				test4_freeFunc(v);
			}
			temp.clear();
		}
		else
		{
			int to = dist3(rng);
			T* newVar = test4_allocFunc<T>();
			if (p.push(index, to, newVar) == false)
			{
				test4_freeFunc(newVar);
			}
		}
	}

	Sleep(100);

	p.pop(index, temp);

	for (auto v : temp)
	{
		test4_freeFunc(v);
	}
	temp.clear();
	TLSObjectPool<T>::release();

	return 0;
}

template <typename T>
int test4()
{
	MainPool<T>& mp = MainPool<T>::getInstance();
	std::vector<std::thread> threads;

	int testCycle = 100;

	
	for (int i = 0; i < testCycle; i++)
	{
		UINT64 startTick = GetTickCount64();

		for (int j = 0; j < test4_thread_size; ++j)
		{
			threads.emplace_back(test4_func<T>, j);
		}


		for (auto& thread : threads) 
		{
			if (thread.joinable()) 
			{
				thread.join();
			}
		}

		if (i % (testCycle / 100) == 0)
		{
			cout << i / (testCycle / 100) << "%\t" 
				<< TLSObjectPool<T>::usingSize() << "\t/\t" 
				<< TLSObjectPool<T>::usableSize() << "\t:\t" 
				<< GetTickCount64() - startTick - 100 << endl;
		}
	}

	cout << TLSObjectPool<T>::usingSize() << "\t/\t" << TLSObjectPool<T>::usableSize() << endl;

	return 0;
}








int main()
{
	test4<TEST4TYPE>();
	
	return 0;
}