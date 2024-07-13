#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"

#include "Includes/Defines.h"
#include "GraphicDefines.hpp"
#include "GfxDevice.h"
#include "GfxImageView.h"

#include "GfxCommandPool.h"

#include "GfxResourceManager.h"

#include "Graphics/ShaderManagement/GfxShader.h"
#include "GfxUniformBuffer.hpp"
#include "GfxStructuredBuffer.h"

#include "glm/glm.hpp"
#include <unordered_map>

struct GfxPipelineLayout
{
    VkPipelineLayout pipelineLayout;
    operator VkPipelineLayout()
    {
        return pipelineLayout;
    }
};

struct GfxPipeline
{
    VkPipeline pipeline;
    operator VkPipeline()
    {
        return pipeline;
    }
};

struct GfxRenderState
{
    VkRenderPass renderPass;
    VkFramebuffer frameBuffer;

    std::optional<glm::vec4> clearValues[8];
};

struct RenderTargetBlendStates
{
    bool blendEnabled = VK_FALSE;
    VkBlendFactor srcColorBlend;
    VkBlendFactor dstColorBlend;
    VkBlendOp     colorBlendOp;

    VkBlendFactor srcAlphaBlend;
    VkBlendFactor dstAlphaBlend;
    VkBlendOp     alphaBlendOp;

    // hash function
    operator uint32_t() const;
};

struct VertexInputState
{
    size_t m_descriptionCount;


    // hash function
    operator uint32_t() const;
};

class GfxPipelineStateManager
{
    DefaultSingleton(GfxPipelineStateManager);
private:
    std::unordered_map<uint64_t, GfxPipeline> m_activePipelines;
    std::unordered_map<uint64_t, GfxRenderState> m_RenderState;
    std::unordered_map<uint64_t, GfxPipelineLayout> m_activePipelineLayout;
    std::optional<GfxImageView> m_RenderTargetImageView[8];

    std::hash<void*> m_hasher;
    GfxDevice* m_device;
    // Pipeline layout variables
    VertexInputState m_vertexState;
    VkPrimitiveTopology m_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPolygonMode m_polygonMode = VK_POLYGON_MODE_FILL;

    RenderTargetBlendStates m_rtBlendStates[8];

    // renderpass variables
    std::optional<glm::vec4> m_clearValues[8];

    GfxPipeline& CreatePipeline(uint32_t hash);
    GfxRenderState& CreateRenderState(uint32_t hash);
    GfxPipelineLayout& CreatePipelineLayout(uint32_t hash);

    struct PipelineMiscInfo
    {
        std::vector<VkDynamicState> dynamicStates;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        VkVertexInputBindingDescription bindingDescription;

        VkPipelineDynamicStateCreateInfo dynamicState;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssembly;

        VkPipelineViewportStateCreateInfo viewportState;

        VkPipelineRasterizationStateCreateInfo rasterizer;

        VkPipelineMultisampleStateCreateInfo multisampling;
        VkPipelineColorBlendAttachmentState colorBlendAttachment[8];

        VkPipelineColorBlendStateCreateInfo colorBlending;
    };

    void FillPipelineCreateMiscInfo(VkGraphicsPipelineCreateInfo& pipelineInfo, PipelineMiscInfo& miscInfo);

    GfxStructuredBuffer* m_structuredBuffer;
    GfxUniformBufferBase* m_uniformBuffer[3];
    VkDescriptorSetLayout m_setLayouts[4];
    uint32_t m_setCount = 0;

    uint32_t shaderCount = 0;
    const GfxShader* m_computeShader = nullptr;
    const GfxShader* m_pixelShader = nullptr;
    const GfxShader* m_vertexShader = nullptr;

public:
    void Init(GfxDevice& device);
    void CleanUp();

    GfxPipeline& GetPipeline();
    GfxRenderState GetRenderState();
    GfxPipelineLayout& GetPipelineLayout();

    void CommitStates(GfxCommandBuffer& commandBuffer);

    void SetVertexInputState(VertexInputState state);
    void SetTopology(VkPrimitiveTopology topology);
    void SetPolygonMode(VkPolygonMode polygonMode);
    void SetRTBlendState(RenderTargetBlendStates blendState, size_t targetRT);

    void SetRTClearvalue(uint32_t rtIndex, glm::vec4 clearValue);
    void SetRenderTarget(uint32_t rtIndex, uint32_t uid)
    {
        m_RenderTargetImageView[rtIndex] = GfxResourceManager::GetImageView(uid);
        m_clearValues[rtIndex] = glm::vec4(0, 0, 0, 0);
    }
    void SetRenderTarget(uint32_t rtIndex, GfxImageView& imageView)
    {
        m_RenderTargetImageView[rtIndex] = imageView;
        m_clearValues[rtIndex] = glm::vec4(0, 0, 0, 0);
    }

    void BindDescriptor(GfxUniformBufferBase& uniformBuffer)
    {
        m_uniformBuffer[uniformBuffer.GetSetIndex() - uniformBuffer.c_uniformBufferOffset] = &uniformBuffer;
        m_setLayouts[uniformBuffer.GetSetIndex()] = uniformBuffer.GetLayout();
    }
    void BindStructuredBuffer(GfxStructuredBuffer& structuredBuffer)
    {
        m_structuredBuffer = &structuredBuffer;
        m_setLayouts[0] = structuredBuffer.GetLayout();
    }

    void ResetRenderTargets();

    void SetShader(const GfxShader& shader);
};

