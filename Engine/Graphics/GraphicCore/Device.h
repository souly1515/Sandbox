#pragma once
#include "Includes/Defines.h"
#include "vulkan/vulkan.h"
#include "Engine/Flag.h"
#include "Surface.h"

#include <optional>

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

class Device
{
  int32_t ScoreDevice(VkPhysicalDevice device, const Surface& surface);
  int32_t IsDeviceUsable(VkPhysicalDevice device);

  std::vector<const char*> PrepareExtensions(VkPhysicalDevice device);

  VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
  VkDevice m_device = VK_NULL_HANDLE;
  VkQueue m_graphicsQueue = VK_NULL_HANDLE;
  VkQueue m_computeQueue = VK_NULL_HANDLE;
  VkQueue m_presentQueue = VK_NULL_HANDLE;

  std::vector<const char*> m_requiredExtensions;
  std::vector<const char*> m_optionalExtensions;
public:
  QueueFamilyIndices GetQueueFamily(VkPhysicalDevice device, const Surface& surface);
  
  operator VkPhysicalDevice() const
  {
    return m_physicalDevice;
  }
  operator VkDevice() const
  {
    return m_device;
  }
  void Init(VkInstance vkInstance, const Surface& surface);

  void RegisterExtensions(const std::vector<const char*>& requiredExtensions, const std::vector<const char*>& optionalExtensions);

  void CleanUp();
};