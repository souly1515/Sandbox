#pragma once

#include "Shared/ShaderIncludes.h"
#include "../autogen/Shaders.h"
#include "Includes/Defines.h"
#include "Graphics/GraphicCore/Device.h"

#include <unordered_map>
#include "vulkan/vulkan.h"

class Shader
{
  VkShaderModule m_shaderModule = VK_NULL_HANDLE;
  Device* m_device;
public:
  ~Shader();
  bool Init(Device& device, const uint32_t* shaderCode, size_t size);
};

class ShaderManager
{
  DefaultSingleton(ShaderManager);
private:
  std::unordered_map<ShaderKey, Shader> m_shaderMap;
  void LoadShaderPerms(ShaderInfo& shaderName, uint32_t baseHash, uint32_t defineFlags, uint32_t numDefines);
  void LoadShaderPerms(ShaderInfo& shaderName, uint32_t baseHash, uint32_t numDefines);
  void CreateShader(ShaderInfo& shaderName, uint32_t hash);

  Device* m_device;
public:
  void Init(Device& device);
  const Shader& GetShader(ShaderKey key) const;
  template<typename ShaderType>
  const Shader& GetShader(ShaderType, ShaderKey defineHash) const
  {
    ShaderKey hash = (defineHash & ShaderDefineMask) | ShaderType::Hash;
    return m_shaderMap.at(hash);
  }
};

