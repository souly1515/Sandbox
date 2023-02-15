#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"

#include "Includes/Defines.h"
#include "GraphicDefines.hpp"

#include "Surface.h"
#include "Device.h"

#include "glm/glm.hpp"

class Swapchain
{
  VkSwapchainKHR m_swapChain;
  Device* m_device = nullptr;
  std::vector<VkImage> m_swapChainImages;
  std::vector<VkSurfaceFormatKHR> m_availableFormats;
  std::vector<VkPresentModeKHR> m_availablePresentModes;
  bool IsFormatAvailable(VkSurfaceFormatKHR format);
  bool IsPresentModeAvailable(VkPresentModeKHR format);
public:
  operator VkSwapchainKHR() const
  {
    return m_swapChain;
  }
  void Init(Device &device, VkSurfaceFormatKHR swapchainFormat, QueueFamilyIndices queueFamily, Surface surface, glm::vec2 size);

  VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, glm::vec2 winSize) const;
  void Cleanup();
};