#pragma once

#include "Shared/ShaderIncludes.h"
#include "../autogen/ShaderData.h"
#include "Includes/Defines.h"
#include "Graphics/GraphicCore/Device.h"
#include "Shader.h"

#include <unordered_map>
#include "vulkan/vulkan.h"

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
  void CleanUp();
  const Shader& GetShader(ShaderKey key) const;
  template<typename ShaderType>
  const Shader& GetShader(ShaderType, ShaderKey defineHash) const
  {
    ShaderKey hash = (defineHash & ShaderDefineMask) | ShaderType::Hash;
    return m_shaderMap.at(hash);
  }
};

