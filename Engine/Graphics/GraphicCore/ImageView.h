#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"

#include "Device.h"

#include "Includes/Defines.h"
#include "GraphicDefines.hpp"


class ImageView
{
  VkImageView m_imageView;
  Device m_device;
public:
  ~ImageView();
  operator VkImageView();
  // maybe put array level at somepoint for image array support
  void Init(VkImage image, VkFormat format, Device& device); 
};