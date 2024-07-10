#pragma once

#include "Shared/ShaderIncludes.h"
#include "../autogen/ShaderData.h"
#include "Includes/Defines.h"
#include "Graphics/GraphicCore/GfxDevice.h"
#include "GfxShader.h"

#include <unordered_map>
#include "vulkan/vulkan.h"

class GfxShaderManager
{
    DefaultSingleton(GfxShaderManager);
private:
    std::unordered_map<ShaderKey, GfxShader> m_shaderMap;
    void LoadShaderPerms(ShaderInfo& shaderName, uint32_t baseHash, uint32_t defineFlags, uint32_t numDefines);
    void LoadShaderPerms(ShaderInfo& shaderName, uint32_t baseHash, uint32_t numDefines);
    void CreateShader(ShaderInfo& shaderName, uint32_t hash);

    GfxDevice* m_device;
public:
    void Init(GfxDevice& device);
    void CleanUp();
    static const GfxShader& GetShader(ShaderKey key);
    template<typename ShaderType>
    const GfxShader& GetShader(ShaderType, ShaderKey defineHash) const
    {
        ShaderKey hash = (defineHash & ShaderDefineMask) | ShaderType::Hash;
        return m_shaderMap.at(hash);
    }
};

