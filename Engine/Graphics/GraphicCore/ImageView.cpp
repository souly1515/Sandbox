#include "ImageView.h"

ImageView::~ImageView()
{
  vkDestroyImageView(m_device, m_imageView, nullptr);
}

ImageView::operator VkImageView()
{
  assert(m_imageView != VK_NULL_HANDLE);
  return m_imageView;
}

void ImageView::Init(VkImage image, VkFormat format, Device& device)
{
  m_device = device;

  VkImageViewCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  createInfo.image = image;
  createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  createInfo.format = format;
  
  createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  createInfo.subresourceRange.baseMipLevel = 0;
  createInfo.subresourceRange.levelCount = 1;
  createInfo.subresourceRange.baseArrayLayer = 0;
  createInfo.subresourceRange.layerCount = 1;

  API_CALL(vkCreateImageView, m_device, &createInfo, nullptr, &m_imageView);
}
