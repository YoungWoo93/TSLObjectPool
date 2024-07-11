#pragma once

#include <vector>
using namespace std;

struct _1k_struct {
	_1k_struct() {};
	~_1k_struct() {};
	char temp[1024 * 1];
};

struct _2k_struct {
	_2k_struct() {};
	~_2k_struct() {};
	char temp[1024 * 2];
};


struct _3k_struct {
	_3k_struct() {};
	~_3k_struct() {};
	char temp[1024 * 3];
};


struct _4k_struct {
	_4k_struct() {};
	~_4k_struct() {};
	char temp[1024 * 4];
};


struct _5k_struct {
	_5k_struct() {};
	~_5k_struct() {};
	char temp[1024 * 5];
};


struct _6k_struct {
	_6k_struct() {};
	~_6k_struct() {};
	char temp[1024 * 6];
};


struct _7k_struct {
	_7k_struct() {};
	~_7k_struct() {};
	char temp[1024 * 7];
};


struct _8k_struct {
	_8k_struct() {};
	~_8k_struct() {};
	char temp[1024 * 8];
};


struct _9k_struct {
	_9k_struct() {};
	~_9k_struct() {};
	char temp[1024 * 9];
};


struct _10k_struct {
	_10k_struct() {};
	~_10k_struct() {};
	char temp[1024 * 10];
};


struct _11k_struct {
	_11k_struct() {};
	~_11k_struct() {};
	char temp[1024 * 11];
};


struct _12k_struct {
	_12k_struct() {};
	~_12k_struct() {};
	char temp[1024 * 12];
};


struct _13k_struct {
	_13k_struct() {};
	~_13k_struct() {};
	char temp[1024 * 13];
};


struct _14k_struct {
	_14k_struct() {};
	~_14k_struct() {};
	char temp[1024 * 14];
};


struct _15k_struct {
	_15k_struct() {};
	~_15k_struct() {};
	char temp[1024 * 15];
};


struct _16k_struct {
	_16k_struct() {};
	~_16k_struct() {};
	char temp[1024 * 16];
};


struct _17k_struct {
	_17k_struct() {};
	~_17k_struct() {};
	char temp[1024 * 17];
};


struct _18k_struct {
	_18k_struct() {};
	~_18k_struct() {};
	char temp[1024 * 18];
};


struct _19k_struct {
	_19k_struct() {};
	~_19k_struct() {};
	char temp[1024 * 19];
};


struct _20k_struct {
	_20k_struct() {};
	~_20k_struct() {};
	char temp[1024 * 20];
};


struct _21k_struct {
	_21k_struct() {};
	~_21k_struct() {};
	char temp[1024 * 21];
};


struct _22k_struct {
	_22k_struct() {};
	~_22k_struct() {};
	char temp[1024 * 22];
};


struct _23k_struct {
	_23k_struct() {};
	~_23k_struct() {};
	char temp[1024 * 23];
};


struct _24k_struct {
	_24k_struct() {};
	~_24k_struct() {};
	char temp[1024 * 24];
};


struct _25k_struct {
	_25k_struct() {};
	~_25k_struct() {};
	char temp[1024 * 25];
};


struct _26k_struct {
	_26k_struct() {};
	~_26k_struct() {};
	char temp[1024 * 26];
};


struct _27k_struct {
	_27k_struct() {};
	~_27k_struct() {};
	char temp[1024 * 27];
};


struct _28k_struct {
	_28k_struct() {};
	~_28k_struct() {};
	char temp[1024 * 28];
};


struct _29k_struct {
	_29k_struct() {};
	~_29k_struct() {};
	char temp[1024 * 29];
};


struct _30k_struct {
	_30k_struct() {};
	~_30k_struct() {};
	char temp[1024 * 30];
};


struct _31k_struct {
	_31k_struct() {};
	~_31k_struct() {};
	char temp[1024 * 31];
};


struct _32k_struct {
	_32k_struct() {};
	~_32k_struct() {};
	char temp[1024 * 32];
};





template <typename T>
class IPC_PIPE
{
public:
	IPC_PIPE()
	{

	}
	IPC_PIPE(int threadSize)
	{
		setThreads(threadSize);
	}

	// 복사 생성자
	IPC_PIPE(const IPC_PIPE& other) : size(other.size) {
		setThreads(size);
	}

	IPC_PIPE(IPC_PIPE&& other) noexcept : size(other.size) {
		other.thread_size = 0;
		setThreads(size);
	}

	// 복사 대입 연산자
	IPC_PIPE& operator=(const IPC_PIPE& other) {
		if (this != &other) {
			size = other.size;
			pipe = other.pipe;
			locks = other.locks;
		}
		return *this;
	}

	// 이동 대입 연산자
	IPC_PIPE& operator=(IPC_PIPE&& other) noexcept {
		if (this != &other) {
			size = other.size;
			pipe = std::move(other.pipe);
			locks = std::move(other.locks);
			other.size = 0;
		}
		return *this;
	}

	void setThreads(int threadSize)
	{
		size = threadSize;
		pipe.assign(size, vector<vector<T*>>(size, vector<T*>()));
		locks.assign(size, vector<SRWLOCK>(size, SRWLOCK_INIT));
	}

	bool push(int myIndex, int targetIndex, T* value)
	{
		AcquireSRWLockExclusive(&locks[targetIndex][myIndex]);

		pipe[targetIndex][myIndex].push_back(value);

		ReleaseSRWLockExclusive(&locks[targetIndex][myIndex]);

		return true;
	}

	void pop(int myIndex, vector<T*>& container)
	{
		for (int i = 0; i < size; i++)
		{
			AcquireSRWLockExclusive(&locks[myIndex][i]);
			for (auto v : pipe[myIndex][i])
			{
				container.push_back(v);
			}
			pipe[myIndex][i].clear();

			ReleaseSRWLockExclusive(&locks[myIndex][i]);
		}

	}
public:
	int size;
	vector<vector<vector<T*>>> pipe;
	vector<vector<SRWLOCK>> locks;
};
