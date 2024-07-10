#pragma once

#include "Shared/ShaderIncludes.h"
#include "Graphics/GraphicCore/GfxDevice.h"

#include "vulkan/vulkan.h"

class GfxShader
{
    VkShaderModule m_shaderModule = VK_NULL_HANDLE;
    GfxDevice* m_device;
    ShaderType m_shaderType;
public:
    ~GfxShader();
    ShaderType GetShaderType() const;
    operator VkShaderModule() const;
    bool Init(GfxDevice& device, ShaderType shaderType, const uint32_t* shaderCode, size_t size);
};
