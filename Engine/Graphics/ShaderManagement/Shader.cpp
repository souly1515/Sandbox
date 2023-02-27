#include "Shader.h"
#include "../autogen/ShaderData.h"
#include "Includes/Defines.h"

Shader::~Shader()
{
  if (m_shaderModule != VK_NULL_HANDLE)
  {
    vkDestroyShaderModule(*m_device, m_shaderModule, nullptr);
  }
}

ShaderType Shader::GetShaderType() const
{
  return m_shaderType;
}

Shader::operator VkShaderModule()
{
  return m_shaderModule;
}

bool Shader::Init(Device& device, ShaderType shaderType, const uint32_t* shaderCode, size_t size)
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
