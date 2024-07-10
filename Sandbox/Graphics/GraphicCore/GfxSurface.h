#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"

#include "Includes/Defines.h"
#include "GraphicDefines.hpp"

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class GfxSurface
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

    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;
};