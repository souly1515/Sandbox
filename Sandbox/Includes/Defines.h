#pragma once

#include <cassert>
#include <cstdio>
#include <stdint.h>
#include <Windows.h>

#define DefaultSingleton(ClassName) \
private:\
  inline static ClassName* ms_instance = nullptr;\
public:\
  static void CreateInstance() { ms_instance = new ClassName { }; }\
  static bool IsCreated() { return ms_instance != nullptr; }\
  static ClassName& GetInstance() { assert(ms_instance); return *ms_instance; }\
  static ClassName* GetInstancePtr() { assert(ms_instance); return ms_instance; }\
  static void DestroyInstance() { delete ms_instance; ms_instance = nullptr; }

#define TemplateSingleton(ClassName, TemplateType) \
private:\
  inline static ClassName<TemplateType>* ms_instance = nullptr;\
public:\
  static void CreateInstance() { ms_instance = new ClassName<TemplateType> { }; }\
  static bool IsCreated() { return ms_instance != nullptr; }\
  static ClassName<TemplateType>& GetInstance() { assert(ms_instance); return *ms_instance; }\
  static void DestroyInstance() { delete ms_instance; ms_instance = nullptr; }

enum MessageSeverity
{
    None = 0,
    Critical = 1,
    Severe = 2,
    Debug = 3,
    Info = 4,
    Verbose = 5,
};

#define UNUSED_PARAM(t) ((void)t)


#ifdef NDEBUG
const uint32_t g_messageLevel = None;
#else
const uint32_t g_messageLevel = Verbose;
#endif

template<typename... Args>
void Log(const char* t, uint32_t logLevel, [[maybe_unused]] Args... args)
{
#ifndef NDEBUG
    if (logLevel <= g_messageLevel)
    {
        char temp[2048];
        sprintf_s(temp, t, args...);
        printf(temp);
        OutputDebugStringA(temp);

    }
#else
    UNUSED_PARAM(t);
    UNUSED_PARAM(logLevel);
#endif

}

#ifdef NDEBUG
#define DEBUG_ONLY(expression)\
(void)(0);
#else
#define DEBUG
#define DEBUG_ONLY(expression)\
expression
#endif

template<typename... Args>
void DebugLog(const char* t, Args... args)
{
    Log(t, Debug, args...);
}
