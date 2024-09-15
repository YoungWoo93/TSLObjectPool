
#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <thread>
#include <vector>
#include <Windows.h>
#include <string>


#include "../TLSObjectPool.h"
#include "lib/profiler.h"

#include "test-Main.h"
#include "lib/boost/pool/singleton_pool.hpp"

thread_local std::mt19937 rng(std::random_device{}());

struct tag {};
template <typename T>
using pool = boost::singleton_pool<tag, sizeof(T)>;

string fileName;
int memorySize;
int threadSize;
int testCase;
int testOption;
int testMode;

using namespace std;


IPC_PIPE<_1k_struct> p1;
IPC_PIPE<_2k_struct> p2;
IPC_PIPE<_3k_struct> p3;
IPC_PIPE<_4k_struct> p4;
IPC_PIPE<_5k_struct> p5;
IPC_PIPE<_6k_struct> p6;
IPC_PIPE<_7k_struct> p7;
IPC_PIPE<_8k_struct> p8;
IPC_PIPE<_9k_struct> p9;
IPC_PIPE<_10k_struct> p10;
IPC_PIPE<_11k_struct> p11;
IPC_PIPE<_12k_struct> p12;
IPC_PIPE<_13k_struct> p13;
IPC_PIPE<_14k_struct> p14;
IPC_PIPE<_15k_struct> p15;
IPC_PIPE<_16k_struct> p16;
IPC_PIPE<_17k_struct> p17;
IPC_PIPE<_18k_struct> p18;
IPC_PIPE<_19k_struct> p19;
IPC_PIPE<_20k_struct> p20;
IPC_PIPE<_21k_struct> p21;
IPC_PIPE<_22k_struct> p22;
IPC_PIPE<_23k_struct> p23;
IPC_PIPE<_24k_struct> p24;
IPC_PIPE<_25k_struct> p25;
IPC_PIPE<_26k_struct> p26;
IPC_PIPE<_27k_struct> p27;
IPC_PIPE<_28k_struct> p28;
IPC_PIPE<_29k_struct> p29;
IPC_PIPE<_30k_struct> p30;
IPC_PIPE<_31k_struct> p31;
IPC_PIPE<_32k_struct> p32;






template <typename T>
T* allocFunc()
{
	switch (testMode)
	{
	case 1:
		return TLSObjectPool<T>::alloc();
		break;
	case 2:
		return new T;
		break;
	case 3:
		return (T*)pool<T>::malloc();
		break;
	case 4:
		return 0;
		break;
	}
}

template <typename T>
void freeFunc(T* v)
{
	switch (testMode)
	{
	case 1:
		TLSObjectPool<T>::free(v);
		break;
	case 2:
		delete v;
		break;
	case 3:
		pool<T>::free(v);
		break;
	case 4:
		break;
	}

}





void parseArguments(int argc, char* argv[]) 
{
	fileName = argv[1];
	try {
		memorySize = std::stoi(argv[2]);	//1~32
		threadSize = std::stoi(argv[3]);	//1~20
		testCase = std::stoi(argv[4]);		//1~6 : testN
		testOption = std::stoi(argv[5]);	//각 테스트의 옵션마다 다름
		testMode = std::stoi(argv[6]);		//1~4 : 1=my, 2=OS, 3=boost, 4=notiong
	}
	catch (const std::invalid_argument& e) {
		throw std::invalid_argument("not integers.");
	}
}

void setIPCModule() {
	p1.setThreads(threadSize);
	p2.setThreads(threadSize);
	p3.setThreads(threadSize);
	p4.setThreads(threadSize);
	p5.setThreads(threadSize);
	p6.setThreads(threadSize);
	p7.setThreads(threadSize);
	p8.setThreads(threadSize);
	p9.setThreads(threadSize);
	p10.setThreads(threadSize);
	p11.setThreads(threadSize);
	p12.setThreads(threadSize);
	p13.setThreads(threadSize);
	p14.setThreads(threadSize);
	p15.setThreads(threadSize);
	p16.setThreads(threadSize);
	p17.setThreads(threadSize);
	p18.setThreads(threadSize);
	p19.setThreads(threadSize);
	p20.setThreads(threadSize);
	p21.setThreads(threadSize);
	p22.setThreads(threadSize);
	p23.setThreads(threadSize);
	p24.setThreads(threadSize);
	p25.setThreads(threadSize);
	p26.setThreads(threadSize);
	p27.setThreads(threadSize);
	p28.setThreads(threadSize);
	p29.setThreads(threadSize);
	p30.setThreads(threadSize);
	p31.setThreads(threadSize);
	p32.setThreads(threadSize);
}



std::string GetResultFilePath() {
	std::string cppPath = __FILE__;

	size_t lastSlashIndex = cppPath.find_last_of("\\/");
	if (std::string::npos != lastSlashIndex)
	{
		return cppPath.substr(0, lastSlashIndex + 1) + "results\\";
	}
	return "";
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// test1
//
// 1스레드, 자가 생산 소멸, 메모리 크기 변화 (1k~32k)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
void test1Function() // 1스레드 생산 본인소멸_크기만 변화
{
	T* t;
	vector<T*> buffer;
	TLSObjectPool<T>::init(3, 2);

	for (int i = 0; i < 1000; i++)
	{
		buffer.push_back(allocFunc<T>());
	}
	for (int i = 0; i < 1000; i++)
	{
		freeFunc<T>(buffer[i]);
	}

	buffer.clear();

	for (int cycle = 0; cycle < 100; cycle++)
	{
		cout << cycle << endl;
		scopeProfiler p(fileName);

		for (int i = 0; i < 100; i++)
		{
			for (int j = 0; j < 1000; j++)
			{
				buffer.push_back(allocFunc<T>());
			}
			for (int j = 0; j < 1000; j++)
			{
				freeFunc<T>(buffer[j]);
			}

			buffer.clear();
		}
	}
	

	TLSObjectPool<T>::release();
}

void runTest1() {
	switch (memorySize) {
	case 1: test1Function<_1k_struct>(); break;
	case 2: test1Function<_2k_struct>(); break;
	case 3: test1Function<_3k_struct>(); break;
	case 4: test1Function<_4k_struct>(); break;
	case 5: test1Function<_5k_struct>(); break;
	case 6: test1Function<_6k_struct>(); break;
	case 7: test1Function<_7k_struct>(); break;
	case 8: test1Function<_8k_struct>(); break;
	case 9: test1Function<_9k_struct>(); break;
	case 10: test1Function<_10k_struct>(); break;
	case 11: test1Function<_11k_struct>(); break;
	case 12: test1Function<_12k_struct>(); break;
	case 13: test1Function<_13k_struct>(); break;
	case 14: test1Function<_14k_struct>(); break;
	case 15: test1Function<_15k_struct>(); break;
	case 16: test1Function<_16k_struct>(); break;
	case 17: test1Function<_17k_struct>(); break;
	case 18: test1Function<_18k_struct>(); break;
	case 19: test1Function<_19k_struct>(); break;
	case 20: test1Function<_20k_struct>(); break;
	case 21: test1Function<_21k_struct>(); break;
	case 22: test1Function<_22k_struct>(); break;
	case 23: test1Function<_23k_struct>(); break;
	case 24: test1Function<_24k_struct>(); break;
	case 25: test1Function<_25k_struct>(); break;
	case 26: test1Function<_26k_struct>(); break;
	case 27: test1Function<_27k_struct>(); break;
	case 28: test1Function<_28k_struct>(); break;
	case 29: test1Function<_29k_struct>(); break;
	case 30: test1Function<_30k_struct>(); break;
	case 31: test1Function<_31k_struct>(); break;
	case 32: test1Function<_32k_struct>(); break;
	default: throw std::invalid_argument("Invalid memory size");
	}
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// test2
//
// 경합 스레드변화 (1~20개), 자가 생산 소멸, 메모리 크기 고정 (옵션1:2k, 옵션2 : 17k)
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SRWLOCK test2Lock;
template <typename T>
void test2ThreadFunction()
{
	T* t;
	vector<T*> buffer;
	TLSObjectPool<T>::init(3, 2);
	
	AcquireSRWLockShared(&test2Lock);
	ReleaseSRWLockShared(&test2Lock);

	for (int i = 0; i < 1000; i++)
	{
		buffer.push_back(allocFunc<T>());
	}
	for (int i = 0; i < 1000; i++)
	{
		freeFunc<T>(buffer[i]);
	}

	buffer.clear();

	for (int cycle = 0; cycle < 10; cycle++)
	{
		scopeProfiler p(fileName);

		for (int i = 0; i < 100; i++)
		{
			for (int j = 0; j < 1000; j++)
			{
				buffer.push_back(allocFunc<T>());
			}
			for (int j = 0; j < 1000; j++)
			{
				freeFunc<T>(buffer[j]);
			}

			buffer.clear();
		}
	}

	TLSObjectPool<T>::release();
}

template <typename T>
void test2Function()
{
	test2Lock = SRWLOCK_INIT;
	vector<thread> threads;

	for (int i = 0; i < 10; i++)
	{
		cout << i * 10 << endl;
		threads.clear();

		AcquireSRWLockExclusive(&test2Lock);
		for (int j = 0; j < threadSize; ++j)
		{
			threads.emplace_back(test2ThreadFunction<T>);
		}

		Sleep(1000);
		ReleaseSRWLockExclusive(&test2Lock);

		for (auto& thread : threads)
		{
			if (thread.joinable())
				thread.join();
		}
	}
}

void runTest2() {
	if(testOption == 1)
		test2Function<_2k_struct>();
	if (testOption == 2)
		test2Function<_17k_struct>();
}





////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// test3
//
// 경합 스레드변화 (1~9개), 생산담당 / 소멸담당 스레드 가변 (옵션1 : 생산 스레드1 고정, 옵션2 : 소멸 스레드 1고정)
// 메모리 크기 고정 (4k)
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SRWLOCK test3Lock1;
SRWLOCK test3Lock2;
void test3ProducerFunction(int myIndex, int targetIndex)
{
	std::uniform_int_distribution<int> dist(1, threadSize - 1);
	_4k_struct* t;
	TLSObjectPool<_4k_struct>::init(3, 2);

	AcquireSRWLockShared(&test3Lock1);
	ReleaseSRWLockShared(&test3Lock1);

	for (int i = 0; i < 100; i++)
	{
		for (int j = 0; j < 1000; j++)
		{
			int target = targetIndex;
			if (target < 0)
				target = dist(rng);

			p4.push(myIndex, target, allocFunc<_4k_struct>());
		}
	}

	TLSObjectPool<_4k_struct>::release();
}


void test3ConsumerFunction(int myIndex)
{
	_4k_struct* t;
	TLSObjectPool<_4k_struct>::init(3, 2);
	vector<_4k_struct*> buffer;
	
	AcquireSRWLockShared(&test3Lock1);
	ReleaseSRWLockShared(&test3Lock1);

	while (!TryAcquireSRWLockExclusive(&test3Lock2))
	{
		for (int j = 0; j < 1000; j++)
		{
			p4.pop(myIndex, buffer);

			for (auto v : buffer)
			{
				freeFunc<_4k_struct>(v);
			}

			buffer.clear();
		}
	}
	ReleaseSRWLockExclusive(&test3Lock2);

	for (int j = 0; j < 1000; j++)
	{
		p4.pop(myIndex, buffer);

		for (auto v : buffer)
		{
			freeFunc<_4k_struct>(v);
		}

		buffer.clear();
	}

	TLSObjectPool<_4k_struct>::release();
}


void test3Function()
{
	test3Lock1 = SRWLOCK_INIT;
	test3Lock2 = SRWLOCK_INIT;
	vector<thread> threads;

	if (testOption == 1)
	{
		for (int i = 0; i < 100; i++)
		{
			cout << i << endl;
			threads.clear();

			AcquireSRWLockExclusive(&test3Lock1);
			AcquireSRWLockExclusive(&test3Lock2);
			threads.emplace_back(test3ConsumerFunction, 0);

			for (int j = 1; j < threadSize; j++)
			{
				threads.emplace_back(test3ProducerFunction, j, 0);
			}

			Sleep(1000);

			scopeProfiler p(fileName);
			ReleaseSRWLockExclusive(&test3Lock1);


			for (int j = 1; j < threadSize; j++)
			{
				if(threads[j].joinable())
					threads[j].join();
			}
			ReleaseSRWLockExclusive(&test3Lock2);

			if (threads[0].joinable())
				threads[0].join();
			
		}
	}
	else if (testOption == 2)
	{
		for (int i = 0; i < 100; i++)
		{
			cout << i<< endl;
			threads.clear();

			AcquireSRWLockExclusive(&test3Lock1);
			AcquireSRWLockExclusive(&test3Lock2);
			threads.emplace_back(test3ProducerFunction, 0, -1);
			
			for (int j = 1; j < threadSize; j++)
			{
				threads.emplace_back(test3ConsumerFunction, j);
			}

			Sleep(1000);

			scopeProfiler p(fileName);
			ReleaseSRWLockExclusive(&test3Lock1);

			if (threads[0].joinable())
				threads[0].join();
			
			ReleaseSRWLockExclusive(&test3Lock2);
			for (int j = 1; j < threadSize; j++)
			{
				if (threads[j].joinable())
				{
					threads[j].join();
				}
			}
		}
	}
}

void runTest3() {
	test3Function();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// test4
//
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <queue>
queue<int*> test4Queue;
SRWLOCK test4Lock;
bool test4Flag = true;

void test4LeakFunction()
{
	TLSObjectPool<int>::init(2, 1);

	std::uniform_int_distribution<int> dist(1, 9);
	int target = dist(rng);

	for (int i = 0; i < target; i++)
	{
		AcquireSRWLockExclusive(&test4Lock);
		test4Queue.push(TLSObjectPool<int>::alloc());
		ReleaseSRWLockExclusive(&test4Lock);
	}
	Sleep(10);

	TLSObjectPool<int>::release();
}


void test4CollcectFunction()
{
	TLSObjectPool<int>::init(1, 1);

	TLSPool<int>* t = TLSObjectPool<int>::objectPool;
	while (test4Flag)
	{
		AcquireSRWLockExclusive(&test4Lock);
		while (!test4Queue.empty())
		{
			TLSObjectPool<int>::free(test4Queue.front());
			test4Queue.pop();
		}
		ReleaseSRWLockExclusive(&test4Lock);
	}

	TLSObjectPool<int>::release();
}

MainPool<int>* test4Main;
void test4Function()
{
	test4Main = &MainPool<int>::getInstance();
	thread collectThread(test4CollcectFunction);
	int testCycle = 2000;
	vector<thread> threads;
	for (int i = 0; i < 2000; i++)
	{
		cout << i << " / " << testCycle << endl;
		cout << "\t useable : " << MainPool<int>::getInstance().usableSize() << endl;
		cout << "\t using : " << MainPool<int>::getInstance().usingSize() << " / " << MainPool<int>::getInstance().collector.size << endl;
		cout << endl;
		threads.clear();

		int threadSize = 1 + rand() % 10;
		for (int j = 0; j < threadSize; j++)
		{
			threads.emplace_back(test4LeakFunction);
		}

		for (int j = 0; j < threadSize; j++)
		{
			if (threads[j].joinable())
				threads[j].join();
		}
	}
	
	test4Flag = false;

	collectThread.join();
}

int main(int argc, char* argv[])
{
	test4Function();

	return 0;
}

/*/
int main(int argc, char* argv[])
{
	parseArguments(argc, argv);
	setIPCModule();

	switch (testCase)
	{
	case 1:runTest1(); break;
	case 2:runTest2(); break;
	case 3:runTest3(); break;
	}

	writeProfiles(GetResultFilePath() + fileName);
	return 0;
}

/*/