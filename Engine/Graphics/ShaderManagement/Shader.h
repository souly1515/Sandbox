#pragma once

#include "Shared/ShaderIncludes.h"
#include "Graphics/GraphicCore/Device.h"

#include "vulkan/vulkan.h"

class Shader
{
  VkShaderModule m_shaderModule = VK_NULL_HANDLE;
  Device* m_device;
  ShaderType m_shaderType;
public:
  ~Shader();
  ShaderType GetShaderType() const;
  operator VkShaderModule();
  bool Init(Device& device, ShaderType shaderType, const uint32_t* shaderCode, size_t size);
};
