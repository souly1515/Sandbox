#include "PlatformManager.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"

#include <iostream>
#include "Graphics/GraphicCore/GraphicDefines.hpp"

void PlatformManager::Init()
{
  glfwInit();
  m_size = { DISPLAY_WIDTH, DISPLAY_HEIGHT };

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  m_window = glfwCreateWindow(DISPLAY_WIDTH, DISPLAY_HEIGHT, "Vulkan window", nullptr, nullptr);
}

void PlatformManager::CleanUp()
{
  glfwDestroyWindow(m_window);

  glfwTerminate();
}

bool PlatformManager::CheckExit()
{
  return glfwWindowShouldClose(m_window);
}

void PlatformManager::HandleIO()
{
  glfwPollEvents();
}

