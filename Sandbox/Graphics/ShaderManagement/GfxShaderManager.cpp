#include "GfxShaderManager.h"
#include "../autogen/ShaderData.h"
#include "Includes/Defines.h"

#include <fstream>
#include <cassert>

void GfxShaderManager::LoadShaderPerms(ShaderInfo& shaderName, uint32_t baseHash, uint32_t defineFlags, uint32_t numDefines)
{
    uint32_t curDefineFlags = (1 << (numDefines - 1) << ShaderDefineBitshift);

    for (uint32_t i = 0; i < numDefines; ++i)
    {
        uint32_t hash = (baseHash & ShaderKeyMask) | ((defineFlags | (curDefineFlags >> i)) & ShaderDefineMask);
        CreateShader(shaderName, hash);

        // recursively initialise all permutations
        LoadShaderPerms(shaderName, baseHash, (defineFlags | (curDefineFlags >> i) & ShaderDefineMask), (numDefines - 1) - i);
    }
}

void GfxShaderManager::LoadShaderPerms(ShaderInfo& shaderName, uint32_t baseHash, uint32_t numDefines)
{
    Log("Loading Shader %s\n", Verbose, std::string(shaderName).c_str());
    CreateShader(shaderName, baseHash);
    LoadShaderPerms(shaderName, baseHash, 0, numDefines);
}

void GfxShaderManager::CreateShader(ShaderInfo& shaderName, uint32_t hash)
{
    std::string shaderFolderPath = "../Bin/Shaders/";
    char shaderFullName[128];
    sprintf_s<128>(shaderFullName, "%s_%08X.spv", std::string(shaderName).c_str(), hash);
    std::string shaderPath = shaderFolderPath + shaderFullName;

    assert(m_shaderMap.find(hash) == m_shaderMap.end());

    GfxShader shader;

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
        m_shaderMap[hash].Init(*m_device, ShaderType(shaderName.GetShaderStage()), reinterpret_cast<const uint32_t*>(buffer.data()), buffer.size());
    }
    catch (...)
    {
        Log("Failed to init Shader %s\n", Severe, shaderFullName);
    }
}

void GfxShaderManager::Init(GfxDevice& device)
{
    m_device = &device;
    for (auto& shaderInfo : g_shaderInfos)
    {
        uint32_t baseHash = (*shaderInfo);

        LoadShaderPerms(*shaderInfo, baseHash, shaderInfo->GetNumDefines());
    }
}

const GfxShader& GfxShaderManager::GetShader(ShaderKey key)
{
    return ms_instance->m_shaderMap.at(key);
}

void GfxShaderManager::CleanUp()
{
    m_shaderMap.clear();
}
