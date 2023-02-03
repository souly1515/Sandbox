#pragma once
#include "Includes/Defines.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class PlatformManager
{
  DefaultSingleton(PlatformManager);
  GLFWwindow* m_window;
public:
  void Init();

  void CleanUp();

  bool CheckExit();

  void HandleIO();

  GLFWwindow* GetWindow()
  {
    return m_window;
  }
};