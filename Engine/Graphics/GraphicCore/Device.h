#pragma once
#include "Includes/Defines.h"
#include "vulkan/vulkan.h"
#include "Engine/Flag.h"
#include "Surface.h"

#include <optional>

class Device
{
  struct QueueFamilyIndices 
  {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> computeFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete() 
    {
      return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
    }
  };

  int ScoreDevice(VkPhysicalDevice device, const Surface& surface);
  VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
  VkDevice m_device = VK_NULL_HANDLE;
  VkQueue m_graphicsQueue = VK_NULL_HANDLE;
  VkQueue m_computeQueue = VK_NULL_HANDLE;
  VkQueue m_presentQueue = VK_NULL_HANDLE;

  QueueFamilyIndices GetQueueFamily(VkPhysicalDevice device, const Surface& surface);
public:
  operator VkPhysicalDevice() const
  {
    return m_physicalDevice;
  }
  operator VkDevice() const
  {
    return m_device;
  }
  void Init(VkInstance vkInstance, const Surface& surface);

  void CleanUp();
};