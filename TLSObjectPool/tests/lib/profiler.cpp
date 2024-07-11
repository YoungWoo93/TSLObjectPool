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
/// 프로파일러가 생성(또는 시작) 된 시점의 해당 스레드를 식별 할 수 있는 key를 이용하여
/// 해당 스레드의 프로파일링 데이터를 저장할 컨테이너 포인터를 TLS로 가져온다
/// </summary>
/// <param name="_key"> 고유 식별자</param>
std::map<std::string, profileStruct>* performanceProfiler::addKey(unsigned long long int _key)
{
	AcquireSRWLockExclusive(&lock);
	if (containors.find(_key) == containors.end()) 
		containors[_key] = std::map<std::string, profileStruct>();
	ReleaseSRWLockExclusive(&lock);

    return &containors[_key];
}

/// <summary>
/// 현재의 tick과 threadID를 이용해 key를 만든다.
/// 이후 addKey를 통해 해당 스레드를 위한 컨테이너를 만든다.
/// </summary>
void profilerInit()
{
    unsigned long long int key = (GetTickCount64() << 32) | (unsigned long long int)GetCurrentThreadId();

    TLS_profileData = PP.addKey(key);
}


/// <summary>
/// 이름 (또는 태그) 를 기준으로 하여 소요 시간을 저장하기 위한 초기화 작업을 진행한다.
/// 만약 동일한 이름(또는 태그)가 기 등록 되어있다면 에러
/// </summary>
/// <param name="name">프로파일링 이름</param>
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
/// 프로파일링 start 지점부터 현재까지 해당하는 이름(또는 태그) 의 소요시간을 계산하여 저장한다
/// 
/// 노이즈를 필터링 하기위해 상위2개, 하위2개 항목은 별도 기록한다.
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
/// CSV 확장자로 파일을 빼기 위한 함수
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
/// 파일 IO
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
