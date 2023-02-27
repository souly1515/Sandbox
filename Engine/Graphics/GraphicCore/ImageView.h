#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"

#include "Device.h"

#include "Includes/Defines.h"
#include "GraphicDefines.hpp"

class Image
{
  VkImage m_imageView;
  Device m_device;

  VkFormat m_format;
public:
  ~Image();
  operator VkImage();
  // maybe put array level at somepoint for image array support
  void Init(VkFormat format, Device& device);
  VkFormat GetFormat();
};

class ImageView
{
  VkImageView m_imageView;
  Device m_device;
  VkFormat m_format;
public:
  VkFormat GetFormat();
  ~ImageView();
  operator VkImageView();
  // maybe put array level at somepoint for image array support
  void Init(VkImage image, VkFormat format, Device& device); 
};