#include "ShaderManager.h"
#include "../autogen/ShaderData.h"
#include "Includes/Defines.h"

#include <fstream>
#include <cassert>

void ShaderManager::LoadShaderPerms(ShaderInfo& shaderName, uint32_t baseHash, uint32_t defineFlags, uint32_t numDefines)
{
  uint32_t curDefineFlags = (1 << (numDefines - 1) << ShaderDefineBitshift);

  for (uint32_t i = 0; i < numDefines; ++i)
  {
    uint32_t hash = (baseHash & ShaderKeyMask) | defineFlags | (curDefineFlags >> i);
    CreateShader(shaderName, hash);

    LoadShaderPerms(shaderName, baseHash, defineFlags | (curDefineFlags >> i), (numDefines - 1) - i);
  }
}

void ShaderManager::LoadShaderPerms(ShaderInfo& shaderName, uint32_t baseHash, uint32_t numDefines)
{
  LoadShaderPerms(shaderName, baseHash, 0, numDefines);
}

void ShaderManager::CreateShader(ShaderInfo& shaderName, uint32_t hash)
{
  std::string shaderFolderPath = "../Bin/Shaders/";
  char shaderFullName[128];
  sprintf_s<128>(shaderFullName, "%s_%08X.spv", std::string(shaderName).c_str(), hash);
  std::string shaderPath = shaderFolderPath + shaderFullName;

  Log("current Iteration: %08X\n", Verbose, hash);
  
  assert(m_shaderMap.find(hash) == m_shaderMap.end());

  Shader shader;

  std::ifstream fs{ shaderPath, std::ios::ate | std::ios::binary };
  if (!fs.is_open())
  {
    Log("Cannot find Shader %s at %s\n", Severe, shaderFullName, shaderPath.c_str());
    return;
  }

  size_t fileSize = (size_t)fs.tellg();
  std::vector<char> buffer(fileSize);
  fs.seekg(0);
  fs.read(buffer.data(), fileSize);
  fs.close();

  try
  {
    shader.Init(*m_device, ShaderType(shaderName.GetShaderStage()), reinterpret_cast<const uint32_t*>(buffer.data()), buffer.size());
  }
  catch (...)
  {
    Log("Failed to init Shader %s\n", Severe, shaderFullName);
  }
  m_shaderMap[hash] = shader;
}

void ShaderManager::Init(Device& device)
{
  m_device = &device;
  for (auto& shaderInfo : g_shaderInfos)
  {
    uint32_t baseHash = (*shaderInfo);

    LoadShaderPerms(*shaderInfo, shaderInfo->GetShaderStage(), baseHash, shaderInfo->GetNumDefines());
  }
}

const Shader& ShaderManager::GetShader(ShaderKey key) const
{
  return m_shaderMap.at(key);
}

void ShaderManager::CleanUp()
{
  m_shaderMap.clear();
}
