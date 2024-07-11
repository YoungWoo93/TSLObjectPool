#pragma once


#include <Windows.h>
#include <string>
#include <map>

struct profileStruct
{
    profileStruct()
    {
        sum = 0;
        min[0] = UINT64_MAX;
        min[1] = UINT64_MAX;
        max[0] = 0;
        max[1] = 0;
        count = 0;
        key = 0;
    }
    LARGE_INTEGER start;
    
    unsigned long long int sum;
    unsigned long long int min[2];
    unsigned long long int max[2];

    unsigned long long int count;
    unsigned long long int key;
};

class profilerException : public std::exception {
public:
    profilerException(const char* msg) {
        strcpy_s(message, 256, msg);
    }
    char* what() {
        return message;
    }

private:
    char message[256];
};

class performanceProfiler
{
public:
    performanceProfiler();
    ~performanceProfiler();
    std::map<std::string, profileStruct>* addKey(unsigned long long int _key);
private:
    SRWLOCK lock;
};


class scopeProfiler
{
public:
    scopeProfiler(const std::string& _name);
    ~scopeProfiler();

private:
    std::string name;
};

void profilerInit();

void startProfile(const std::string& name);
void endProfile(const std::string& name);
void clearProfile();

void writeProfiles();
void writeProfiles(const std::string& _dir);