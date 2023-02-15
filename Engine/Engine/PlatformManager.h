#pragma once
#include "Includes/Defines.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

class PlatformManager
{
  DefaultSingleton(PlatformManager);
  GLFWwindow* m_window;
  glm::vec2 m_size;
public:
  void Init();

  glm::vec2 GetSize() const
  {
    return m_size;
  }

  void CleanUp();

  bool CheckExit();

  void HandleIO();

  GLFWwindow* GetWindow()
  {
    return m_window;
  }
};