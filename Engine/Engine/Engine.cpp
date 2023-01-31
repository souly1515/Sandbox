#include "Engine.h"

#include "PlatformManager.h"
#include "Graphics/GraphicCore/GraphicEngine.h"

#include <iostream>

int Engine::MainLoop()
{
  PlatformManager& pm = PlatformManager::GetInstance();

  while (!pm.CheckExit())
  {
    pm.HandleIO();
  }

  return m_returnValue;
}

void Engine::PrintShit()
{
  std::cout << "printing shit" << std::endl;
}

void Engine::Init()
{
  PlatformManager::CreateInstance();
  GraphicEngine::CreateInstance();

  PlatformManager::GetInstance().Init();
  GraphicEngine::GetInstance().Init();
}

void Engine::CleanUp()
{
  PlatformManager::GetInstance().CleanUp();
}



