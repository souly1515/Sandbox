#include "Device.h"
#include "GraphicDefines.hpp"

#include <vector>
#include <map>
#include <set>

void Device::Init(VkInstance vkInstance, const Surface& surface)
{
  uint32_t deviceCount = 0;

  API_CALL(vkEnumeratePhysicalDevices, vkInstance, &deviceCount, nullptr);
  if (deviceCount == 0)
  {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }
  std::vector<VkPhysicalDevice> devices(deviceCount);
  API_CALL(vkEnumeratePhysicalDevices, vkInstance, &deviceCount, devices.data());
  // Use an ordered map to automatically sort candidates by increasing score
  std::multimap<int, VkPhysicalDevice> candidates;

  for (const auto& device : devices)
  {
    int score = ScoreDevice(device, surface);
    if (score)
      candidates.insert(std::make_pair(score, device));
  }

  if (candidates.rbegin()->first > 0)
  {
    m_physicalDevice = candidates.rbegin()->second;
  }
  else
  {
    throw std::runtime_error("failed to find a suitable GPU!");
  }

  QueueFamilyIndices queueFamilies = GetQueueFamily(m_physicalDevice, surface);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
  std::set<uint32_t> uniqueQueueFamilies = { 
    queueFamilies.graphicsFamily.value(), 
    queueFamilies.presentFamily.value(), 
    queueFamilies.computeFamily.value() };

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies)
  {
    VkDeviceQueueCreateInfo queueCreateInfo{};

    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;

    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }
  VkPhysicalDeviceFeatures deviceFeatures{};

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.queueCreateInfoCount = queueCreateInfos.size();

  createInfo.pEnabledFeatures = &deviceFeatures;

  API_CALL(vkCreateDevice, m_physicalDevice, &createInfo, nullptr, &m_device);

  API_CALL(vkGetDeviceQueue, m_device, queueFamilies.graphicsFamily.value(), 0, &m_graphicsQueue);
  API_CALL(vkGetDeviceQueue, m_device, queueFamilies.presentFamily.value(), 0, &m_presentQueue);
  API_CALL(vkGetDeviceQueue, m_device, queueFamilies.computeFamily.value(), 0, &m_computeQueue);
}

void Device::CleanUp()
{
  API_CALL(vkDestroyDevice, m_device, nullptr);
}


Device::QueueFamilyIndices Device::GetQueueFamily(VkPhysicalDevice device, const Surface& surface)
{
  QueueFamilyIndices indices;
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
  int i = 0;

  for (const auto& queueFamily : queueFamilies)
  {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
      indices.graphicsFamily = i;

    if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
      indices.computeFamily = i;

    VkBool32 presentSupport = false;
    API_CALL(vkGetPhysicalDeviceSurfaceSupportKHR, device, i, surface, &presentSupport);

    if (presentSupport)
      indices.presentFamily = i;

    if (indices.isComplete())
      break;
    i++;
  }

  return indices;
}

int Device::ScoreDevice(VkPhysicalDevice device, const Surface& surface)
{
  VkPhysicalDeviceProperties deviceProperties;
  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
  int score = 0;

  // Discrete GPUs have a significant performance advantage
  if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    score += 1000;

  score += deviceProperties.limits.maxImageDimension2D;

  QueueFamilyIndices queueFamily = GetQueueFamily(device, surface);

  if (!queueFamily.isComplete())
    score = 0;

  return score;
}