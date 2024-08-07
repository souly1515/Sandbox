#include "GfxSwapchain.h"
#include "GfxDevice.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <algorithm>

bool GfxSwapChain::IsFormatAvailable(VkSurfaceFormatKHR format)
{
    for (const auto& availableFormat : m_availableFormats) {
        if (availableFormat.format == format.format && availableFormat.colorSpace == format.colorSpace) {
            return true;
        }
    }
    return false;
}

bool GfxSwapChain::IsPresentModeAvailable(VkPresentModeKHR format)
{
    for (const auto& availablePresentMode : m_availablePresentModes) {
        if (availablePresentMode == format) {
            return true;
        }
    }

    return false;
}

void GfxSwapChain::Init(GfxDevice& device, VkSurfaceFormatKHR swapchainFormat, const GfxSurface& surface, glm::vec2 size)
{
    SwapChainSupportDetails swapChainSupport = surface.QuerySwapChainSupport(device);
    QueueFamilyIndices queueFamily = device.GetQueueFamily();

    m_device = &device;
    m_availableFormats = swapChainSupport.formats;
    m_availablePresentModes = swapChainSupport.presentModes;

    assert(IsFormatAvailable(swapchainFormat));


    VkSurfaceFormatKHR surfaceFormat = swapchainFormat;
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

    if (IsPresentModeAvailable(VK_PRESENT_MODE_MAILBOX_KHR))
        presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, size);
    m_extent = { extent.width, extent.height };

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
    m_swapChainImageViews.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); i++)
    {
        m_swapChainImageViews[i].Init(m_swapChainImages[i], surfaceFormat.format, *m_device);
    }
    m_numFrames = (uint8_t)imageCount;
}

VkExtent2D GfxSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, glm::vec2 winSize) const
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

void GfxSwapChain::Cleanup()
{
    m_swapChainImageViews.clear();
    API_CALL(vkDestroySwapchainKHR, *m_device, m_swapChain, nullptr);
}

VkFormat GfxSwapChain::GetSwapChainFormat() const
{
    return m_swapChainImageViews[0].GetFormat();
}
