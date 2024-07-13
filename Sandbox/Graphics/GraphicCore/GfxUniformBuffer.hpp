#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"

#include "Includes/Defines.h"
#include "GraphicDefines.hpp"
#include "GfxDevice.h"
#include "GfxBuffer.h"
#include "GfxDescriptorPool.h"

class GfxUniformBufferBase
{
public:
    static const uint32_t c_uniformBufferOffset = 1;
    virtual VkDescriptorSetLayout GetLayout() = 0;
    virtual constexpr uint32_t GetSetIndex() = 0;

    virtual operator VkDescriptorSet* () = 0;

    virtual operator VkDescriptorSet& () = 0;
};

template<typename Data, uint32_t SetIndex = 0>
class GfxUniformBuffer : public GfxUniformBufferBase
{
    static GfxDevice* s_device;
    static VkDescriptorSetLayout s_descriptorSetLayout;
    VkDescriptorSet m_descriptorSet;

    GfxBuffer m_buffer;
    void* m_gpuMem = nullptr;
public:
    Data m_data;

    static void CreateLayout(GfxDevice& device);

    VkDescriptorSetLayout GetLayout() override
    {
        return s_descriptorSetLayout;
    }

    operator VkDescriptorSet* () override
    {
        return &m_descriptorSet;
    }
    operator VkDescriptorSet& () override
    {
        return m_descriptorSet;
    }

    constexpr uint32_t GetSetIndex() override
    {
        return SetIndex + c_uniformBufferOffset;
    }

    void CreateBuffer()
    {
        m_buffer.CreateBuffer(*s_device, sizeof(Data), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        GfxDescriptorPool& descPool = GfxDescriptorPool::GetInstance();

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descPool.GetUniformPool();
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &s_descriptorSetLayout;

        vkAllocateDescriptorSets(*s_device, &allocInfo, &m_descriptorSet);

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(Data);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_descriptorSet;
        descriptorWrite.dstBinding = 0;

        descriptorWrite.dstArrayElement = 0; 
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;

        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(*s_device, 1, &descriptorWrite, 0, nullptr);

        m_gpuMem = m_buffer.Map();
    }
    void UpdateBuffer()
    {
        memcpy(m_gpuMem, &m_data, sizeof(Data));
    }

    void CleanUp()
    {
        m_buffer.Unmap();
        m_buffer.CleanUp();
    }

    static void CleanUpLayouts();
};


template<typename Data, uint32_t SetIndex>
void GfxUniformBuffer<Data, SetIndex>::CreateLayout(GfxDevice& device)
{
    s_device = &device;
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    API_CALL(vkCreateDescriptorSetLayout, device, &layoutInfo, nullptr, &s_descriptorSetLayout);

}

template<typename Data, uint32_t SetIndex>
void GfxUniformBuffer<Data, SetIndex>::CleanUpLayouts()
{
    API_CALL(vkDestroyDescriptorSetLayout, *s_device, s_descriptorSetLayout, nullptr);
}

template<typename Data, uint32_t SetIndex>
GfxDevice* GfxUniformBuffer<Data, SetIndex>::s_device = nullptr;

template<typename Data, uint32_t SetIndex>
VkDescriptorSetLayout GfxUniformBuffer<Data, SetIndex>::s_descriptorSetLayout;
