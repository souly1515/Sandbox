#include "GfxPipelineStateManager.h"
#include "GfxObjectManager.h"
#include "GfxVertex.h"

GfxPipelineLayout& GfxPipelineStateManager::CreatePipelineLayout(uint32_t hash)
{
    GfxPipelineLayout pipelineLayout;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = m_setCount;
    pipelineLayoutInfo.pSetLayouts = m_setLayouts;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    API_CALL(vkCreatePipelineLayout, *m_device, &pipelineLayoutInfo, nullptr, &pipelineLayout.pipelineLayout);

    m_activePipelineLayout.insert({ hash, pipelineLayout });
    return m_activePipelineLayout.at(hash);
}

void GfxPipelineStateManager::CommitStates(GfxCommandBuffer& commandBuffer)
{
    API_CALL(vkCmdBindPipeline, commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GetPipeline());

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GetPipelineLayout(), 0, 1, (*m_structuredBuffer), 0, nullptr);
    for (int i = 0; i < 3; ++i)
    {
        if (!m_uniformBuffer[i])
            break;
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GetPipelineLayout(), i + 1, 1, *(m_uniformBuffer[i]), 0, nullptr);
    }
}

GfxPipeline& GfxPipelineStateManager::CreatePipeline(uint32_t hash)
{
    GfxPipeline pipe = {};

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    assert((m_vertexShader && m_pixelShader) || m_computeShader);
    VkPipelineShaderStageCreateInfo shaderStages[2] = {};
    if (m_vertexShader)
    {
        pipelineInfo.stageCount = 2;

        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = *m_vertexShader;
        shaderStages[0].pName = "main";

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = *m_pixelShader;
        shaderStages[1].pName = "main";
    }
    else
    {
        pipelineInfo.stageCount = 1;

        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStages[0].module = *m_computeShader;
        shaderStages[0].pName = "main";
    }
    pipelineInfo.pStages = shaderStages;

    GfxPipelineLayout layout = GetPipelineLayout();
    pipelineInfo.layout = layout.pipelineLayout;

    GfxRenderState renderstate = GetRenderState();
    pipelineInfo.renderPass = renderstate.renderPass;
    pipelineInfo.subpass = 0;

    PipelineMiscInfo pipelineMisc = {};

    FillPipelineCreateMiscInfo(pipelineInfo, pipelineMisc);

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    API_CALL(vkCreateGraphicsPipelines, *m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipe.pipeline);

    m_activePipelines.emplace(hash, pipe);
    return m_activePipelines.at(hash);
}

void GfxPipelineStateManager::FillPipelineCreateMiscInfo(VkGraphicsPipelineCreateInfo& pipelineInfo, PipelineMiscInfo& pipelineMisc)
{
    pipelineMisc.dynamicStates =
    {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR
    };

    pipelineMisc.dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pipelineMisc.dynamicState.dynamicStateCount = static_cast<uint32_t>(pipelineMisc.dynamicStates.size());
    pipelineMisc.dynamicState.pDynamicStates = pipelineMisc.dynamicStates.data();

    
    pipelineMisc.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO; 
    pipelineMisc.bindingDescription = GfxStandardVertex::GetBindingDescription();
    pipelineMisc.attributeDescriptions = GfxStandardVertex::GetAttributeDescriptions();

    pipelineMisc.vertexInputInfo.vertexBindingDescriptionCount = 1;
    pipelineMisc.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(pipelineMisc.attributeDescriptions.size());
    pipelineMisc.vertexInputInfo.pVertexBindingDescriptions = &pipelineMisc.bindingDescription;
    pipelineMisc.vertexInputInfo.pVertexAttributeDescriptions = pipelineMisc.attributeDescriptions.data();

    pipelineMisc.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineMisc.inputAssembly.topology = m_topology;
    pipelineMisc.inputAssembly.primitiveRestartEnable = VK_FALSE;

    pipelineMisc.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipelineMisc.viewportState.viewportCount = 1;
    pipelineMisc.viewportState.scissorCount = 1;

    pipelineMisc.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipelineMisc.rasterizer.depthClampEnable = VK_FALSE;
    pipelineMisc.rasterizer.rasterizerDiscardEnable = VK_FALSE;
    pipelineMisc.rasterizer.polygonMode = m_polygonMode;
    pipelineMisc.rasterizer.lineWidth = 1.0f;
    pipelineMisc.rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    pipelineMisc.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    pipelineMisc.rasterizer.depthBiasEnable = VK_FALSE;
    pipelineMisc.rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    pipelineMisc.rasterizer.depthBiasClamp = 0.0f; // Optional
    pipelineMisc.rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    pipelineMisc.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipelineMisc.multisampling.sampleShadingEnable = VK_FALSE;
    pipelineMisc.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineMisc.multisampling.minSampleShading = 1.0f; // Optional
    pipelineMisc.multisampling.pSampleMask = nullptr; // Optional
    pipelineMisc.multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    pipelineMisc.multisampling.alphaToOneEnable = VK_FALSE; // Optional

    uint32_t attachmentCount = 0;
    for (; attachmentCount < 8; ++attachmentCount)
    {
        pipelineMisc.colorBlendAttachment[attachmentCount] = {};
        if (!m_RenderTargetImageView[attachmentCount].has_value())
            break;
        if (m_rtBlendStates[attachmentCount].blendEnabled)
        {
            pipelineMisc.colorBlendAttachment[attachmentCount].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            pipelineMisc.colorBlendAttachment[attachmentCount].blendEnable = VK_TRUE;
            pipelineMisc.colorBlendAttachment[attachmentCount].srcColorBlendFactor = m_rtBlendStates[attachmentCount].srcColorBlend;
            pipelineMisc.colorBlendAttachment[attachmentCount].dstColorBlendFactor = m_rtBlendStates[attachmentCount].dstColorBlend;
            pipelineMisc.colorBlendAttachment[attachmentCount].colorBlendOp = m_rtBlendStates[attachmentCount].colorBlendOp;
            pipelineMisc.colorBlendAttachment[attachmentCount].srcAlphaBlendFactor = m_rtBlendStates[attachmentCount].srcAlphaBlend;
            pipelineMisc.colorBlendAttachment[attachmentCount].dstAlphaBlendFactor = m_rtBlendStates[attachmentCount].dstAlphaBlend;
            pipelineMisc.colorBlendAttachment[attachmentCount].alphaBlendOp = m_rtBlendStates[attachmentCount].alphaBlendOp;
        }
        else
        {
            pipelineMisc.colorBlendAttachment[attachmentCount].blendEnable = VK_FALSE;
        }
    }

    pipelineMisc.colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pipelineMisc.colorBlending.logicOpEnable = VK_FALSE;
    pipelineMisc.colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    pipelineMisc.colorBlending.attachmentCount = attachmentCount;
    pipelineMisc.colorBlending.pAttachments = pipelineMisc.colorBlendAttachment;
    pipelineMisc.colorBlending.blendConstants[0] = 0.0f; // Optional
    pipelineMisc.colorBlending.blendConstants[1] = 0.0f; // Optional
    pipelineMisc.colorBlending.blendConstants[2] = 0.0f; // Optional
    pipelineMisc.colorBlending.blendConstants[3] = 0.0f; // Optional

    pipelineInfo.pVertexInputState = &pipelineMisc.vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &pipelineMisc.inputAssembly;
    pipelineInfo.pViewportState = &pipelineMisc.viewportState;
    pipelineInfo.pRasterizationState = &pipelineMisc.rasterizer;
    pipelineInfo.pMultisampleState = &pipelineMisc.multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &pipelineMisc.colorBlending;
    pipelineInfo.pDynamicState = &pipelineMisc.dynamicState;
}

GfxRenderState& GfxPipelineStateManager::CreateRenderState(uint32_t hash)
{
    VkAttachmentDescription colorAttachment[8] = {};
    uint32_t attachmentCount = 0;
    VkImageView attachments[8] = {};
    for (; attachmentCount < 8; ++attachmentCount)
    {
        if (!m_RenderTargetImageView[attachmentCount].has_value())
            break;
        colorAttachment[attachmentCount].format = m_RenderTargetImageView[attachmentCount]->GetFormat();
        colorAttachment[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;

        // TODO: handle clearing properly
        if (m_RenderTargetImageView[attachmentCount].has_value())
        {
            colorAttachment[attachmentCount].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment[attachmentCount].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment[attachmentCount].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[attachmentCount] = *m_RenderTargetImageView[attachmentCount];
        }
        else
        {
            colorAttachment[attachmentCount].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment[attachmentCount].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment[attachmentCount].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        }
        // TODO: handle more then 1 render pass, so final layout cannot be present
        // and initial layout probably cannot be undefined
        colorAttachment[attachmentCount].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment[attachmentCount].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // TODO: do this properly at some point
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1; // probably need to set this to attachment count
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};

    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    GfxRenderState renderState = {};

    API_CALL(vkCreateRenderPass, *m_device, &renderPassInfo, nullptr, &renderState.renderPass);

    GfxSwapChain& swapChain = m_device->GetSwapChain();
    // TODO get frame buffer from resource manager
    // manage it in some way other then having 3 LOL

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderState.renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = swapChain.GetExtent().x;
    framebufferInfo.height = swapChain.GetExtent().y;
    framebufferInfo.layers = 1;

    API_CALL(vkCreateFramebuffer, *m_device, &framebufferInfo, nullptr, &(renderState.frameBuffer));

    m_RenderState.emplace(hash, renderState);

    return m_RenderState.at(hash);
}

GfxPipelineLayout& GfxPipelineStateManager::GetPipelineLayout()
{
    uint32_t hash = 0;
    m_setCount = 1;

    for (int i = 0; i < 3; ++i)
    {
        if (!m_uniformBuffer[i])
            break;
        hash |= m_hasher(m_setLayouts[i]);
        ++m_setCount;
    }
    auto pipeLayoutSearchRes = m_activePipelineLayout.find(hash);

    if (pipeLayoutSearchRes == m_activePipelineLayout.end())
        return CreatePipelineLayout(hash);
    else
        return pipeLayoutSearchRes->second;
}

void GfxPipelineStateManager::Init(GfxDevice& device)
{
    m_device = &device;
}

void GfxPipelineStateManager::CleanUp()
{
    for (auto& pipeline : m_activePipelines)
    {
        API_CALL(vkDestroyPipeline, *m_device, pipeline.second.pipeline, nullptr);
    }
    for (auto& pipeline : m_activePipelineLayout)
    {
        API_CALL(vkDestroyPipelineLayout, *m_device, pipeline.second.pipelineLayout, nullptr);
    }
    m_activePipelines.clear();
    m_activePipelineLayout.clear();
    // destroy here since to destroy this resource, you need access to the device.
    for (auto& rp : m_RenderState)
    {
        API_CALL(vkDestroyRenderPass, *m_device, rp.second.renderPass, nullptr);
        API_CALL(vkDestroyFramebuffer, *m_device, rp.second.frameBuffer, nullptr);
    }
    m_RenderState.clear();
}

GfxPipeline& GfxPipelineStateManager::GetPipeline()
{
    // TODO: hash pipeline layouts and pipelines
    uint32_t renderPassHash = 0;
    for (int i = 0; i < sizeof(m_RenderTargetImageView); ++i)
    {
        if (m_RenderTargetImageView[i].has_value())
        {
            renderPassHash ^= m_hasher(m_RenderTargetImageView[i]->GetVkImage());
        }
        else
        {
            break;
        }
    }

    uint32_t pipelineHash = 0;
    pipelineHash ^= m_hasher((void*)m_vertexShader);
    pipelineHash ^= m_hasher((void*)m_pixelShader);
    pipelineHash ^= m_hasher((void*)m_computeShader);

    uint32_t hash = pipelineHash + renderPassHash;

    auto pipe = m_activePipelines.find(hash);
    if (pipe != m_activePipelines.end())
        return (pipe->second);
    // pipeline does not exist at this point

    return CreatePipeline(hash);
}

GfxRenderState GfxPipelineStateManager::GetRenderState()
{
    // TODO: hash render passes
    uint32_t renderPassHash = 0;
    for (int i = 0; i < sizeof(m_RenderTargetImageView); ++i)
    {
        if (m_RenderTargetImageView[i].has_value())
        {
            renderPassHash ^= m_hasher(m_RenderTargetImageView[i]->GetVkImage());
        }
        else
        {
            break;
        }
    }
    auto rp = m_RenderState.find(renderPassHash);
    if (rp != m_RenderState.end())
    {
        GfxRenderState ret = rp->second;
        for (uint32_t i = 0; i < 8; ++i)
        {
            if (m_clearValues[i].has_value())
                ret.clearValues[i] = m_clearValues[i];
            else
                break;
        }
        return ret;
    }

    GfxRenderState ret = CreateRenderState(renderPassHash);

    for (uint32_t i = 0; i < 8; ++i)
    {
        if (m_clearValues[i].has_value())
            ret.clearValues[i] = m_clearValues[i];
        else
            break;
    }

    return ret;
}

void GfxPipelineStateManager::SetVertexInputState(VertexInputState state)
{
    m_vertexState = state;
}

void GfxPipelineStateManager::SetTopology(VkPrimitiveTopology topology)
{
    m_topology = topology;
}

void GfxPipelineStateManager::SetPolygonMode(VkPolygonMode polygonMode)
{
    m_polygonMode = polygonMode;
}

void GfxPipelineStateManager::SetRTBlendState(RenderTargetBlendStates blendState, size_t targetRT)
{
    m_rtBlendStates[targetRT] = blendState;
}

void GfxPipelineStateManager::SetRTClearvalue(uint32_t rtIndex, glm::vec4 clearValue)
{
    UNUSED_PARAM(rtIndex);
    UNUSED_PARAM(clearValue);
}

void GfxPipelineStateManager::ResetRenderTargets()
{
    for (int i = 0; i < 8; ++i)
    {
        m_RenderTargetImageView[i].reset();
        m_rtBlendStates[i].blendEnabled = false;
        m_clearValues[i].reset();
    }
}

void GfxPipelineStateManager::SetShader(const GfxShader& shader)
{
    switch (shader.GetShaderType())
    {
    case ShaderType::CS:
        m_pixelShader = nullptr;
        m_vertexShader = nullptr;
        m_computeShader = &shader;
        break;
    case ShaderType::PS:
        m_pixelShader = &shader;
        m_computeShader = nullptr;
        break;
    case ShaderType::VS:
        m_vertexShader = &shader;
        m_computeShader = nullptr;
        break;
    }
}

VertexInputState::operator uint32_t() const
{
    return uint32_t(std::hash<uint32_t>{}(uint32_t(m_descriptionCount)));
}

RenderTargetBlendStates::operator uint32_t() const
{
    if (!blendEnabled)
        return 0;
    return 0;
}
