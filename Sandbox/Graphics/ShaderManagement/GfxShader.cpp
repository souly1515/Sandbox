#include "GfxShader.h"
#include "../autogen/ShaderData.h"
#include "Includes/Defines.h"

GfxShader::~GfxShader()
{
    if (m_shaderModule != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(*m_device, m_shaderModule, nullptr);
    }
}

ShaderType GfxShader::GetShaderType() const
{
    return m_shaderType;
}

GfxShader::operator VkShaderModule() const
{
    return m_shaderModule;
}

bool GfxShader::Init(GfxDevice& device, ShaderType shaderType, const uint32_t* shaderCode, size_t size)
{
    m_device = &device;
    m_shaderType = shaderType;

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode = shaderCode;

    API_CALL(vkCreateShaderModule, device, &createInfo, nullptr, &m_shaderModule);

    return true;
}
