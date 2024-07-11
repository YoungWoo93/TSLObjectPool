#include "profiler.h"

#include <Windows.h>
#include <stdio.h>
#include <time.h>

#include <stdexcept>
#include <string>
#include <map>
#include <vector>


LARGE_INTEGER Freq;
std::string dir;
performanceProfiler PP;

std::map<unsigned long long int, std::map<std::string, profileStruct>> containors;
__declspec(thread) std::map<std::string, profileStruct>* TLS_profileData = nullptr;



scopeProfiler::scopeProfiler(const std::string& _name) :name(_name)
{
    startProfile(name);

};

scopeProfiler::~scopeProfiler()
{
    endProfile(name);
};

performanceProfiler::performanceProfiler()
{
    QueryPerformanceFrequency(&Freq);
}

performanceProfiler::~performanceProfiler()
{

}



/// <summary>
/// �������Ϸ��� ����(�Ǵ� ����) �� ������ �ش� �����带 �ĺ� �� �� �ִ� key�� �̿��Ͽ�
/// �ش� �������� �������ϸ� �����͸� ������ �����̳� �����͸� TLS�� �����´�
/// </summary>
/// <param name="_key"> ���� �ĺ���</param>
std::map<std::string, profileStruct>* performanceProfiler::addKey(unsigned long long int _key)
{
	AcquireSRWLockExclusive(&lock);
	if (containors.find(_key) == containors.end()) 
		containors[_key] = std::map<std::string, profileStruct>();
	ReleaseSRWLockExclusive(&lock);

    return &containors[_key];
}

/// <summary>
/// ������ tick�� threadID�� �̿��� key�� �����.
/// ���� addKey�� ���� �ش� �����带 ���� �����̳ʸ� �����.
/// </summary>
void profilerInit()
{
    unsigned long long int key = (GetTickCount64() << 32) | (unsigned long long int)GetCurrentThreadId();

    TLS_profileData = PP.addKey(key);
}


/// <summary>
/// �̸� (�Ǵ� �±�) �� �������� �Ͽ� �ҿ� �ð��� �����ϱ� ���� �ʱ�ȭ �۾��� �����Ѵ�.
/// ���� ������ �̸�(�Ǵ� �±�)�� �� ��� �Ǿ��ִٸ� ����
/// </summary>
/// <param name="name">�������ϸ� �̸�</param>
void startProfile(const std::string& name)
{
    if (TLS_profileData == nullptr)
        profilerInit();

    if (TLS_profileData->find(name) == TLS_profileData->end())
    {
        profileStruct temp;
        temp.sum = 0;
        temp.min[0] = ULLONG_MAX;
        temp.min[1] = ULLONG_MAX;
        temp.max[0] = 0;
        temp.max[1] = 0;
        temp.count = 0;
        (*TLS_profileData)[name] = temp;
    }
    else if ((*TLS_profileData)[name].start.QuadPart != 0)
    {
        std::string msg = "profile name overlapped : " + name;

        throw profilerException(msg.c_str());
    }
    LARGE_INTEGER* p = &((*TLS_profileData)[name].start);
    QueryPerformanceCounter(p);
}

/// <summary>
/// �������ϸ� start �������� ������� �ش��ϴ� �̸�(�Ǵ� �±�) �� �ҿ�ð��� ����Ͽ� �����Ѵ�
/// 
/// ����� ���͸� �ϱ����� ����2��, ����2�� �׸��� ���� ����Ѵ�.
/// </summary>
/// <param name="name"></param>
void endProfile(const std::string& name)
{
    LARGE_INTEGER end;
    QueryPerformanceCounter(&end);

    if ((*TLS_profileData)[name].start.QuadPart == 0)
        return;

    unsigned long long time = end.QuadPart - (*TLS_profileData)[name].start.QuadPart;
    if ((*TLS_profileData)[name].min[1] > time)
    {
        (*TLS_profileData)[name].min[1] = time;
        if ((*TLS_profileData)[name].min[0] > time)
        {
            (*TLS_profileData)[name].min[1] = (*TLS_profileData)[name].min[0];
            (*TLS_profileData)[name].min[0] = time;
        }
    }

    if ((*TLS_profileData)[name].max[1] < time)
    {
        (*TLS_profileData)[name].max[1] = time;
        if ((*TLS_profileData)[name].max[0] < time)
        {
            (*TLS_profileData)[name].max[1] = (*TLS_profileData)[name].max[0];
            (*TLS_profileData)[name].max[0] = time;
        }
    }

    (*TLS_profileData)[name].sum += time;
    (*TLS_profileData)[name].start.QuadPart = 0;
    (*TLS_profileData)[name].count++;
}

void clearProfile()
{
    TLS_profileData->clear();
}





/// <summary>
/// CSV Ȯ���ڷ� ������ ���� ���� �Լ�
/// </summary>
/// <param name="v"></param>
/// <param name="str"></param>
void CSVstringify(const std::vector<std::string>& v, std::string& str)
{
    for (auto _str : v)
    {
        if (_str.find(',') != std::string::npos || _str.find('\"') != std::string::npos)
        {
            str += '\"';
            for (auto character : _str)
            {
                if (character == '\"')
                    str += '\"';

                str += character;
            }
            str += '\"';
            str += ",";
        }
        else
        {
            str += _str;
            str += ",";
        }
    }
    str += "\n";
}


/// <summary>
/// ���� IO
/// </summary>
void writeProfiles()
{
    struct tm curr_tm;
    time_t t = time(0);
    localtime_s(&curr_tm, &t);

    FILE* file;
    std::string filename = ("["
        + std::to_string(curr_tm.tm_year + 1900) + "-"
        + std::to_string(curr_tm.tm_mon + 1) + "-"
        + std::to_string(curr_tm.tm_mday) + "] "
        + std::to_string(curr_tm.tm_hour) + "_"
        + std::to_string(curr_tm.tm_min) + "_"
        + std::to_string(curr_tm.tm_sec)
        + ".csv"
        );

    fopen_s(&file, filename.c_str(), "wt");


    for (auto containor : containors)
    {
        int no = 0;
        fprintf(file, "UniqueID,threadID,No,Name,Min(micro sec),Average(micro sec),Max(micro sec),Call Count\n");
        unsigned long long int uniqueID = containor.first;
        unsigned int threadID = (unsigned int)containor.first;
        for (auto item : containor.second)
        {
            std::vector<std::string> v = {
                std::to_string(uniqueID),
                std::to_string(threadID),
                std::to_string(no++) ,
                item.first ,
                std::to_string((double)item.second.min[1] * 1000 * 1000 / Freq.QuadPart) ,
                std::to_string(((double)(item.second.sum - item.second.min[0] - item.second.max[0]) * 1000 * 1000) / (item.second.count - 2) / Freq.QuadPart),
                std::to_string((double)item.second.max[1] * 1000 * 1000 / Freq.QuadPart),
                std::to_string(item.second.count)
            };

            std::string str;
            CSVstringify(v, str);
            fprintf(file, str.c_str());
        }
        fprintf(file, ",,,,,,,\n");
    }

    fclose(file);
}

void writeProfiles(const std::string& _dir)
{
    struct tm curr_tm;
    time_t t = time(0);
    localtime_s(&curr_tm, &t);

    FILE* file;
    std::string filename = _dir + ".csv";
    fopen_s(&file, filename.c_str(), "wt");


    for (auto containor : containors)
    {
        int no = 0;
        fprintf(file, "UniqueID,threadID,No,Name,Min(micro sec),Average(micro sec),Max(micro sec),Call Count\n");
        unsigned long long int uniqueID = containor.first;
        unsigned int threadID = (unsigned int)containor.first;
        for (auto item : containor.second)
        {
            std::vector<std::string> v = {
                std::to_string(uniqueID),
                std::to_string(threadID),
                std::to_string(no++) ,
                item.first ,
                std::to_string((double)item.second.min[1] * 1000 * 1000 / Freq.QuadPart) ,
                std::to_string(((double)(item.second.sum - item.second.min[0] - item.second.max[0]) * 1000 * 1000) / (item.second.count - 2) / Freq.QuadPart),
                std::to_string((double)item.second.max[1] * 1000 * 1000 / Freq.QuadPart),
                std::to_string(item.second.count)
            };

            std::string str;
            CSVstringify(v, str);
            fprintf(file, str.c_str());
        }
        fprintf(file, ",,,,,,,\n");
    }

    fclose(file);
}
