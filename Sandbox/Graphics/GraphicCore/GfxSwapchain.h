#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"

#include "Includes/Defines.h"
#include "GraphicDefines.hpp"

#include "GfxSurface.h"
#include "GfxImageView.h"

#include "glm/glm.hpp"

class GfxSwapChain
{
    VkSwapchainKHR m_swapChain;
    GfxDevice* m_device = nullptr;
    std::vector<VkImage> m_swapChainImages;
    std::vector<GfxImageView> m_swapChainImageViews;
    std::vector<VkSurfaceFormatKHR> m_availableFormats;
    std::vector<VkPresentModeKHR> m_availablePresentModes;
    bool IsFormatAvailable(VkSurfaceFormatKHR format);
    bool IsPresentModeAvailable(VkPresentModeKHR format);
    glm::u32vec2 m_extent;
    uint32_t m_currentImageIndex;
    uint8_t m_numFrames;
public:
    operator VkSwapchainKHR() const
    {
        return m_swapChain;
    }
    void Init(GfxDevice& device, VkSurfaceFormatKHR swapchainFormat, const GfxSurface& surface, glm::vec2 size);
    VkFormat GetSwapChainFormat() const;

    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, glm::vec2 winSize) const;
    glm::u32vec2 GetExtent() const { return m_extent; };
    VkExtent2D GetVkExtent() const { return { m_extent.x, m_extent.y }; };
    std::vector <GfxImageView>& GetImageViews() { return m_swapChainImageViews; };
    size_t GetNumImageViews() const { return m_numFrames; };

    GfxImageView& GetCurrentImageView() { return m_swapChainImageViews[m_currentImageIndex]; };

    void SetCurrentImageIndex(uint32_t imageIndex) { m_currentImageIndex = imageIndex; };
    uint32_t& GetCurrentImageIndex() { return m_currentImageIndex; };

    void Cleanup();
};