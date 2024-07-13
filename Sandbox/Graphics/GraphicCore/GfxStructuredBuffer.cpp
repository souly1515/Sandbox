#include "GfxStructuredBuffer.h"

void GfxStructuredBuffer::CreateLayout(GfxDevice& device)
{
    m_device = &device;
    VkDescriptorSetLayoutBinding sboLayoutBinding{};
    sboLayoutBinding.binding = 0;
    sboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    sboLayoutBinding.descriptorCount = 1;
    sboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    sboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &sboLayoutBinding;

    API_CALL(vkCreateDescriptorSetLayout, device, &layoutInfo, nullptr, &m_descriptorSetLayout);
}

void GfxStructuredBuffer::CreateBuffer(uint32_t size)
{
    m_buffer.CreateBuffer(*m_device, size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    GfxDescriptorPool& descPool = GfxDescriptorPool::GetInstance();

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descPool.GetStoragePool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_descriptorSetLayout;

    vkAllocateDescriptorSets(*m_device, &allocInfo, &m_descriptorSet);

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = size;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 0;

    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrite.descriptorCount = 1;

    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pImageInfo = nullptr; // Optional
    descriptorWrite.pTexelBufferView = nullptr; // Optional

    vkUpdateDescriptorSets(*m_device, 1, &descriptorWrite, 0, nullptr);

    m_gpuMem = m_buffer.Map();
}

void GfxStructuredBuffer::UpdateBuffer(void* data, size_t src_offset, size_t size)
{
    memcpy(m_gpuMem, (uint8_t*)data + src_offset, size);
}

void GfxStructuredBuffer::CleanUp()
{
    m_buffer.Unmap();
    m_buffer.CleanUp();
}

void GfxStructuredBuffer::CleanUpLayouts()
{
    API_CALL(vkDestroyDescriptorSetLayout, *m_device, m_descriptorSetLayout, nullptr);
}
