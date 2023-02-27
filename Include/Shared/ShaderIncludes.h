#pragma once
#include <cstdint>
#include <string>

using ShaderKey = uint32_t;
const uint32_t ShaderDefineBitshift = 16;
const uint32_t ShaderKeyMask = 0x0000FFFF;
const uint32_t ShaderDefineMask = 0xFFFF0000;

enum class ShaderType
{
  VS = 0,
  PS = 1,
  CS = 2,
};


#include "../vulkan/vulkan.h"
constexpr VkShaderStageFlagBits ShaderTypeToVulkanStage[] =
{
  VK_SHADER_STAGE_VERTEX_BIT,
  VK_SHADER_STAGE_FRAGMENT_BIT,
  VK_SHADER_STAGE_COMPUTE_BIT,
};


inline ShaderKey CombineShaderKey(ShaderKey mainKey, ShaderKey defineKey)
{
  return mainKey | (defineKey << ShaderDefineBitshift);
}


struct ShaderInfo {
  virtual operator uint32_t() = 0;
  virtual operator std::string() = 0;
  virtual uint32_t GetNumDefines() = 0;
  virtual uint32_t GetShaderStage() = 0;
};
