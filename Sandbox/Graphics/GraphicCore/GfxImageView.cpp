#include "GfxImageView.h"
#include "GfxDevice.h"

#include <iostream>
VkFormat GfxImageView::GetFormat() const
{
    return m_format;
}

GfxImageView::~GfxImageView()
{
    if(m_device && m_imageView)
        vkDestroyImageView(*m_device, m_imageView, nullptr);
}

GfxImageView::operator VkImageView()
{
    assert(m_imageView != VK_NULL_HANDLE);
    return m_imageView;
}
RegisterResource_Macro(GfxImageView, TestImageView);
RegisterResource_Macro(GfxImage, TestImage);

void GfxImageView::Init(GfxImage& image, VkFormat format, GfxDevice& device)
{
    m_gfxImage = &image;
    Init((VkImage)image, format, device);
}

void GfxImageView::Init(VkImage image, VkFormat format, GfxDevice& device)
{
    m_device = &device;
    GetResourceType(GfxImageView, TestImageView) temp;
    GetResourceType(GfxImage, TestImage) temp2;
    std::cout << "testing uid_base - " << temp.resourceName << " : " << temp.internalID << std::endl;
    std::cout << "testing uid_base - " << temp2.resourceName << " : " << temp2.internalID << std::endl;

    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    m_format = createInfo.format = format;

    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    m_vkImage = image;

    API_CALL(vkCreateImageView, *m_device, &createInfo, nullptr, &m_imageView);
}

GfxImage::operator VkImage()
{
    return m_imageView;
}

VkFormat GfxImage::GetFormat() const
{
    return m_format;
}
