#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"

#include "glm/glm.hpp"
#include <array>

#include "GfxDevice.h"

#include "GfxCommandPool.h"


class GfxBuffer
{
    VkBuffer m_buffer;
    VkDeviceMemory m_bufferMemory;
    GfxDevice* m_device;
    size_t m_size = 0;

    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
public:
    operator VkBuffer () { return m_buffer; };
    void CreateBuffer(GfxDevice& device, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProperties);
    void CleanUp();
    size_t GetSize() { return m_size; };

    void* Map(size_t size = 0 , size_t offset = 0);

    void Unmap();

    void CopyTo(GfxBuffer& otherBuffer, const GfxCommandBuffer& buffer, size_t size = 0, size_t src_offset = 0, size_t dst_offset = 0);
};
