#pragma once
#include "Includes/Defines.h"


class Engine
{
    DefaultSingleton(Engine);

    int m_returnValue = 0;
    bool m_running = true;
public:

    int MainLoop();

    void PrintShit();

    void Init();
    void CleanUp();
};
