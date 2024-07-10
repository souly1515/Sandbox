#include "GfxCommandPool.h"

void GfxCommandPool::Init(GfxDevice& device)
{
    m_device = &device;
    QueueFamilyIndices queueFamilyIndices = device.GetQueueFamily();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    API_CALL(vkCreateCommandPool, device, &poolInfo, nullptr, &m_graphicsCommandPool);

    poolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily.value();
    API_CALL(vkCreateCommandPool, device, &poolInfo, nullptr, &m_computeCommandPool);

}

void GfxCommandPool::CleanUp()
{
    for (auto& commandAllocator : m_commandBufferAllocators)
    {
        commandAllocator.CleanUp(m_graphicsCommandPool, m_computeCommandPool);
    }

    API_CALL(vkDestroyCommandPool, *m_device, m_graphicsCommandPool, nullptr);
    API_CALL(vkDestroyCommandPool, *m_device, m_computeCommandPool, nullptr);
}

void GfxCommandPool::FrameFlip()
{
    m_currentFrame = -1;
}

void GfxCommandPool::SetCurrentFrame(uint32_t frame)
{
    m_currentFrame = frame;
    if (m_commandBufferAllocators.size() <= frame)
        m_commandBufferAllocators.resize(frame + 1, {m_device});
    else
        ReleaseCommandBuffers(frame);
}

void GfxCommandPool::ReleaseCommandBuffers(uint32_t frame)
{
    m_commandBufferAllocators[frame].ReuseCommandBuffer();
}

void GfxCommandPool::SubmitGraphics()
{
    m_commandBufferAllocators[m_currentFrame].SubmitGraphics();
}

void GfxCommandPool::SubmitCompute()
{
    m_commandBufferAllocators[m_currentFrame].SubmitCompute();
}

GfxCommandBuffer GfxCommandPool::GetGraphicsCommandBuffer()
{
    return m_commandBufferAllocators[m_currentFrame].GetGraphicsCommandBuffer(m_graphicsCommandPool);
}
GfxCommandBuffer GfxCommandPool::GetComputeCommandBuffer()
{
    return m_commandBufferAllocators[m_currentFrame].GetGraphicsCommandBuffer(m_computeCommandPool);
}

GfxCommandBuffer::GfxCommandBuffer(VkCommandBuffer commandBuffer) :
    m_commandBuffer(commandBuffer)
{
}

void GfxCommandBuffer::StartRecording()
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    API_CALL(vkBeginCommandBuffer, m_commandBuffer, &beginInfo);
    m_open = true;
}

void GfxCommandBuffer::EndRecording()
{
    API_CALL(vkEndCommandBuffer, m_commandBuffer);
    m_commandBuffer = VK_NULL_HANDLE; // safety so that it will not be reused here
    m_open = false;
}

GfxCommandBuffer GfxCommandPool::GfxCommandBufferAllocator::GetGraphicsCommandBuffer(VkCommandPool pool)
{
    if (m_graphicsCommandBuffers.size() > m_usedGraphics)
    {
        GfxCommandBuffer cb = m_graphicsCommandBuffers[m_usedGraphics++];
        m_graphicsCommandBuffersInUse.emplace_back(cb);
        return cb;
    }

    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    API_CALL(vkAllocateCommandBuffers, *m_device, &allocInfo, &commandBuffer);

    m_graphicsCommandBuffers.emplace_back(commandBuffer);
    m_graphicsCommandBuffersInUse.emplace_back(commandBuffer);
    ++m_usedGraphics;

    return GfxCommandBuffer(commandBuffer);
}

GfxCommandBuffer GfxCommandPool::GfxCommandBufferAllocator::GetComputeCommandBuffer(VkCommandPool pool)
{
    if (m_computeCommandBuffers.size() > m_usedCompute)
    {
        GfxCommandBuffer cb = m_computeCommandBuffers[m_usedGraphics++];
        m_computeCommandBuffersInUse.emplace_back(cb);
        return cb;
    }

    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    API_CALL(vkAllocateCommandBuffers, *m_device, &allocInfo, &commandBuffer);

    m_computeCommandBuffers.emplace_back(commandBuffer);
    m_computeCommandBuffersInUse.emplace_back(commandBuffer);
    ++m_usedCompute;

    return GfxCommandBuffer(commandBuffer);
}

void GfxCommandPool::GfxCommandBufferAllocator::SubmitGraphics()
{
    m_graphicsCommandBuffersInUse.clear();
}

void GfxCommandPool::GfxCommandBufferAllocator::SubmitCompute()
{
    m_computeCommandBuffersInUse.clear();
}

void GfxCommandPool::GfxCommandBufferAllocator::ReuseCommandBuffer()
{
    for (auto& commandBuffer : m_computeCommandBuffers)
    {
        vkResetCommandBuffer(commandBuffer, 0);
    }

    for (auto& commandBuffer : m_graphicsCommandBuffers)
    {
        vkResetCommandBuffer(commandBuffer, 0);
    }

    m_usedCompute = m_usedGraphics = 0;
}

void GfxCommandPool::GfxCommandBufferAllocator::CleanUp(VkCommandPool graphicsCommandPool, VkCommandPool computeCommandPool)
{
    if (m_graphicsCommandBuffers.size())
    {
        API_CALL(vkFreeCommandBuffers, *m_device, graphicsCommandPool, (uint32_t)m_graphicsCommandBuffers.size(), m_graphicsCommandBuffers.data());
    }
    if (m_computeCommandBuffers.size())
    {
        API_CALL(vkFreeCommandBuffers, *m_device, computeCommandPool, (uint32_t)m_computeCommandBuffers.size(), m_computeCommandBuffers.data());
    }
}
