#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"

#include "Device.h"

#include "Includes/Defines.h"
#include "GraphicDefines.hpp"
#include "Engine/UID.h"


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
  uint32_t GetUID();
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
  uint32_t GetUID();

  // maybe put array level at somepoint for image array support
  void Init(VkImage image, VkFormat format, Device& device);
};