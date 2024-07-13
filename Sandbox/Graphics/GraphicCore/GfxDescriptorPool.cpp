#include "GfxDevice.h"

#include <iostream>
#include "GfxDescriptorPool.h"


VkDescriptorPool GfxDescriptorPool::GetUniformPool()
{
    return m_uniformPool;
}

VkDescriptorPool GfxDescriptorPool::GetStoragePool()
{
    return m_storagePool;
}

void GfxDescriptorPool::InitPools(GfxDevice& device)
{
    m_device = &device;
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 512;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 256;

    API_CALL(vkCreateDescriptorPool, device, &poolInfo, nullptr, &m_uniformPool);

    poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    API_CALL(vkCreateDescriptorPool, device, &poolInfo, nullptr, &m_storagePool);
}

void GfxDescriptorPool::CleanUp()
{
    vkDestroyDescriptorPool(*m_device, m_uniformPool, nullptr);
    vkDestroyDescriptorPool(*m_device, m_storagePool, nullptr);
}
