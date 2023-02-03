#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"

#include "Includes/Defines.h"
#include "GraphicDefines.hpp"

class Surface
{
  VkSurfaceKHR m_surface = VK_NULL_HANDLE;
  VkInstance m_instance = VK_NULL_HANDLE;
public:
  operator VkSurfaceKHR() const
  {
    return m_surface;
  }
  void Init(VkInstance instance);
  void Cleanup();
};