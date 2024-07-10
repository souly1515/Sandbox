#include "GfxSurface.h"
#include "Engine/PlatformManager.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

void GfxSurface::Init(VkInstance instance)
{
    m_instance = instance;
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = glfwGetWin32Window(PlatformManager::GetInstance().GetWindow());
    createInfo.hinstance = GetModuleHandle(nullptr);

    API_CALL(vkCreateWin32SurfaceKHR, m_instance, &createInfo, nullptr, &m_surface);
}

void GfxSurface::Cleanup()
{
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
}

SwapChainSupportDetails GfxSurface::QuerySwapChainSupport(VkPhysicalDevice device) const
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

    uint32_t formatCount;
    API_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR, device, m_surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        API_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR, device, m_surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    API_CALL(vkGetPhysicalDeviceSurfacePresentModesKHR, device, m_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        API_CALL(vkGetPhysicalDeviceSurfacePresentModesKHR, device, m_surface, &presentModeCount, details.presentModes.data());
    }
    return details;
}
