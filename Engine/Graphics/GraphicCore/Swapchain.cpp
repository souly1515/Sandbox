#include "Swapchain.h"
#include "Engine/PlatformManager.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <algorithm>

bool Swapchain::IsFormatAvailable(VkSurfaceFormatKHR format)
{
  for (const auto& availableFormat : m_availableFormats) {
    if (availableFormat.format == format.format && availableFormat.colorSpace == format.colorSpace) {
      return true;
    }
  }
  return false;
}

bool Swapchain::IsPresentModeAvailable(VkPresentModeKHR format)
{
  for (const auto& availablePresentMode : m_availablePresentModes) {
    if (availablePresentMode == format) {
      return true;
    }
  }

  return false;
}

void Swapchain::Init(Device &device, VkSurfaceFormatKHR swapchainFormat, QueueFamilyIndices queueFamily, Surface surface, glm::vec2 size)
{
  SwapChainSupportDetails swapChainSupport = surface.QuerySwapChainSupport(device);
  m_device = &device;
  m_availableFormats = swapChainSupport.formats;
  m_availablePresentModes = swapChainSupport.presentModes;
  
  assert(IsFormatAvailable(swapchainFormat));


  VkSurfaceFormatKHR surfaceFormat = swapchainFormat;
  VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

  if (IsPresentModeAvailable(VK_PRESENT_MODE_MAILBOX_KHR))
    presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

  VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, size);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
  {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = VK_NULL_HANDLE;
  uint32_t queueFamilyIndices[] = { queueFamily.graphicsFamily.value(), queueFamily.presentFamily.value(),  queueFamily.computeFamily.value() };

  if (queueFamily.graphicsFamily != queueFamily.presentFamily && queueFamily.graphicsFamily != queueFamily.computeFamily)
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 3;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  }
  else 
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0; // Optional
    createInfo.pQueueFamilyIndices = nullptr; // Optional
  }



  API_CALL(vkCreateSwapchainKHR, *m_device, &createInfo, nullptr, &m_swapChain);

  API_CALL(vkGetSwapchainImagesKHR, *m_device, m_swapChain, &imageCount, nullptr);
  m_swapChainImages.resize(imageCount);
  API_CALL(vkGetSwapchainImagesKHR, *m_device, m_swapChain, &imageCount, m_swapChainImages.data());
}

VkExtent2D Swapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, glm::vec2 winSize) const
{
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
  {
    return capabilities.currentExtent;
  }
  else 
  {
    VkExtent2D actualExtent = {
        static_cast<uint32_t>(winSize.x),
        static_cast<uint32_t>(winSize.y)
    };

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
  }
}

void Swapchain::Cleanup()
{
  API_CALL(vkDestroySwapchainKHR, *m_device, m_swapChain, nullptr);
}
