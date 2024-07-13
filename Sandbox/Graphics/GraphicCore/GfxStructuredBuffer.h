#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"

#include "Includes/Defines.h"
#include "GraphicDefines.hpp"
#include "GfxDevice.h"
#include "GfxBuffer.h"
#include "GfxDescriptorPool.h"

class GfxStructuredBuffer
{
    GfxDevice* m_device;
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkDescriptorSet m_descriptorSet;

    GfxBuffer m_buffer;
    void* m_gpuMem;
public:

    void CreateLayout(GfxDevice& device);

    VkDescriptorSetLayout GetLayout()
    {
        return m_descriptorSetLayout;
    }

    operator VkDescriptorSet* ()
    {
        return &m_descriptorSet;
    }
    operator VkDescriptorSet& ()
    {
        return m_descriptorSet;
    }

    void CreateBuffer(uint32_t size);

    void UpdateBuffer(void* data, size_t src_offset, size_t size);

    void CleanUp();

    void CleanUpLayouts();
};
