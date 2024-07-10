#pragma once
#include "vulkan/vulkan.h"

#include "GfxDevice.h"

// a weak pointer style handle for command buffers
// will be invalid after closing/stopping recording
class GfxCommandBuffer
{
    VkCommandBuffer m_commandBuffer;
    bool m_open = false;
public:
    GfxCommandBuffer(VkCommandBuffer commandBuffer);

    operator VkCommandBuffer() const
    {
        return m_commandBuffer;
    }

    operator bool() const
    {
        return m_commandBuffer != VK_NULL_HANDLE;
    }

    bool IsOpen() const
    {
        return m_open;
    }

    void StartRecording();
    void EndRecording();
};

class GfxCommandPool
{
    class GfxCommandBufferAllocator
    {
        std::vector<VkCommandBuffer> m_graphicsCommandBuffers;
        std::vector<VkCommandBuffer> m_graphicsCommandBuffersInUse;

        std::vector<VkCommandBuffer> m_computeCommandBuffers;
        std::vector<VkCommandBuffer> m_computeCommandBuffersInUse;
        uint32_t m_usedGraphics = 0;
        uint32_t m_usedCompute = 0;

        GfxDevice* m_device;
    public:
        GfxCommandBufferAllocator(GfxDevice* device) : m_device{ device } {}

        GfxCommandBuffer GetGraphicsCommandBuffer(VkCommandPool pool);
        GfxCommandBuffer GetComputeCommandBuffer(VkCommandPool pool);

        const std::vector<VkCommandBuffer>& GetInUseGraphicsCommandBuffers() const { return m_graphicsCommandBuffersInUse; };
        const std::vector<VkCommandBuffer>& GetInUseComputeCommandBuffers() const { return m_computeCommandBuffersInUse; };

        void SubmitGraphics();
        void SubmitCompute();

        void ReuseCommandBuffer();
        void CleanUp(VkCommandPool graphicsCommandPool, VkCommandPool computeCommandPool);
    };

    VkCommandPool m_graphicsCommandPool;
    VkCommandPool m_computeCommandPool;

    GfxDevice* m_device;
    std::vector<GfxCommandBufferAllocator> m_commandBufferAllocators;
    int32_t m_currentFrame = -1;
public:
    void Init(GfxDevice& device);
    void CleanUp();

    void FrameFlip();

    void SetCurrentFrame(uint32_t frame);

    void ReleaseCommandBuffers(uint32_t frame);

    void SubmitGraphics();
    void SubmitCompute();

    const std::vector<VkCommandBuffer>& GetCurrentGraphicsCommandBuffers() const { return m_commandBufferAllocators[m_currentFrame].GetInUseGraphicsCommandBuffers(); };
    const std::vector<VkCommandBuffer>& GetCurrentComputeCommandBuffers() const { return m_commandBufferAllocators[m_currentFrame].GetInUseComputeCommandBuffers(); };


    GfxCommandBuffer GetGraphicsCommandBuffer();
    GfxCommandBuffer GetComputeCommandBuffer();
};
