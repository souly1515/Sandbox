#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"

#include "Includes/Defines.h"
#include "GraphicDefines.hpp"
#include "Engine/UID.h"

class GfxDevice;

class GfxImage
{
    VkImage m_imageView;
    GfxDevice* m_device;

    VkFormat m_format;
public:
    ~GfxImage();
    operator VkImage();
    // maybe put array level at somepoint for image array support
    void Init(VkFormat format, GfxDevice& device);
    VkFormat GetFormat() const;
};

class GfxImageView
{
    VkImageView m_imageView;
    GfxDevice* m_device;
    VkFormat m_format;
    GfxImage* m_gfxImage = nullptr;
    VkImage m_vkImage;
public:
    ~GfxImageView();
    operator VkImageView();

    // maybe put array level at some point for image array support
    void Init(VkImage image, VkFormat format, GfxDevice& device);
    void Init(GfxImage& image, VkFormat format, GfxDevice& device);

    VkImage GetVkImage() { return m_vkImage; };


    VkFormat GetFormat() const;
    uint32_t GetUID() const;
};