#include "GfxBuffer.h"

uint32_t GfxBuffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(*m_device, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void GfxBuffer::CreateBuffer(GfxDevice& device, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProperties)
{
    m_device = &device;
    m_size = size;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    API_CALL(vkCreateBuffer, device, &bufferInfo, nullptr, &m_buffer);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(*m_device, m_buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, memProperties);
    
    API_CALL(vkAllocateMemory, device, &allocInfo, nullptr, &m_bufferMemory);

    vkBindBufferMemory(device, m_buffer, m_bufferMemory, 0);
}

void GfxBuffer::CleanUp()
{
    vkDestroyBuffer(*m_device, m_buffer, nullptr);
    vkFreeMemory(*m_device, m_bufferMemory, nullptr);
}

void* GfxBuffer::Map(size_t size, size_t offset)
{
    // map full range if unspecified
    if (size == 0)
        size = m_size;

    void* data;
    vkMapMemory(*m_device, m_bufferMemory, offset, size, 0, &data);

    return data;
}

void GfxBuffer::Unmap()
{
    vkUnmapMemory(*m_device, m_bufferMemory);
}

void GfxBuffer::CopyTo(GfxBuffer& dstBuffer, const GfxCommandBuffer& commandBuffer, size_t size, size_t src_offset, size_t dst_offset)
{
    if (size == 0)
    {
        size = m_size;
    }

    assert(size <= dstBuffer.GetSize());

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = src_offset;
    copyRegion.dstOffset = dst_offset;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, m_buffer, dstBuffer, 1, &copyRegion);
}
