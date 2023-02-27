#include "PipelineStateManager.h"

PipelineLayout& PipelineStateManager::CreatePipelineLayout(uint32_t hash)
{
  PipelineLayout pipelineLayout;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0; // Optional
  pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
  pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
  pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

  API_CALL(vkCreatePipelineLayout, *m_device, &pipelineLayoutInfo, nullptr, &pipelineLayout.pipelineLayout);

  m_activePipelineLayout.insert({ hash, pipelineLayout });
  return m_activePipelineLayout.at(hash);
}

Pipeline& PipelineStateManager::CreatePipeline(uint32_t hash)
{
  Pipeline pipe;

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

  PipelineLayout layout = GetPipelineLayout();
  pipelineInfo.layout = layout.pipelineLayout;

  RenderPass renderpass = GetRenderpass();
  pipelineInfo.renderPass = renderpass.renderPass;
  pipelineInfo.subpass = 0;

  FillPipelineCreateMiscInfo(pipelineInfo);

  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
  pipelineInfo.basePipelineIndex = -1; // Optional

  API_CALL(vkCreateGraphicsPipelines, *m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipe.pipeline);

  m_activePipelines.insert({ hash, pipe });
  return m_activePipelines.at(hash);
}

void PipelineStateManager::FillPipelineCreateMiscInfo(VkGraphicsPipelineCreateInfo& pipelineInfo)
{
  std::vector<VkDynamicState> dynamicStates =
  {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
  };

  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicState.pDynamicStates = dynamicStates.data();

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
  vertexInputInfo.vertexAttributeDescriptionCount = 0;
  vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = m_topology;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = m_polygonMode;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f; // Optional
  rasterizer.depthBiasClamp = 0.0f; // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f; // Optional
  multisampling.pSampleMask = nullptr; // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
  multisampling.alphaToOneEnable = VK_FALSE; // Optional
  VkPipelineColorBlendAttachmentState colorBlendAttachment[8] = {};
  for (size_t i = 0; i < 8; ++i)
  {
    colorBlendAttachment[i] = {};
    if (m_rtBlendStates[i].blendEnabled)
    {
      colorBlendAttachment[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
      colorBlendAttachment[i].blendEnable = VK_TRUE;
      colorBlendAttachment[i].srcColorBlendFactor = m_rtBlendStates[i].srcColorBlend;
      colorBlendAttachment[i].dstColorBlendFactor = m_rtBlendStates[i].dstColorBlend;
      colorBlendAttachment[i].colorBlendOp = m_rtBlendStates[i].colorBlendOp;
      colorBlendAttachment[i].srcAlphaBlendFactor = m_rtBlendStates[i].srcAlphaBlend;
      colorBlendAttachment[i].dstAlphaBlendFactor = m_rtBlendStates[i].dstAlphaBlend;
      colorBlendAttachment[i].alphaBlendOp = m_rtBlendStates[i].alphaBlendOp;
    }
    else
    {
      colorBlendAttachment[i].blendEnable = false;
    }
  }

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
  colorBlending.attachmentCount = 8;
  colorBlending.pAttachments = colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f; // Optional
  colorBlending.blendConstants[1] = 0.0f; // Optional
  colorBlending.blendConstants[2] = 0.0f; // Optional
  colorBlending.blendConstants[3] = 0.0f; // Optional

  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = nullptr; // Optional
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
}

RenderPass& PipelineStateManager::CreateRenderpass(uint32_t hash)
{
  VkAttachmentDescription colorAttachment[8];
  uint32_t attachmentCount = 0;
  for (; attachmentCount < 8; ++attachmentCount)
  {
    if (!m_RenderTargetImageView[attachmentCount].has_value())
      break;
    colorAttachment[attachmentCount].format = m_RenderTargetImageView[attachmentCount]->GetFormat();
    colorAttachment[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;

    // TODO: handle clearing properly
    if (m_clearValues[attachmentCount].has_value())
    {
      colorAttachment[attachmentCount].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
      colorAttachment[attachmentCount].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
      colorAttachment[attachmentCount].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      colorAttachment[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
    else
    {
      colorAttachment[attachmentCount].loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      colorAttachment[attachmentCount].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
      colorAttachment[attachmentCount].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      colorAttachment[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
    // TODO: handle more then 1 render pass, so final layout cannot be present
    // and inital layout probably cannot be undefined
    colorAttachment[attachmentCount].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment[attachmentCount].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  }
  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // TODO: do this properly at some point
  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;

  RenderPass renderPass;
  
  API_CALL(vkCreateRenderPass, *m_device, &renderPassInfo, nullptr, &renderPass.renderPass);

  m_activeRenderpass.insert({ hash, renderPass });

  return m_activeRenderpass.at(hash);
}

PipelineLayout& PipelineStateManager::GetPipelineLayout()
{
  uint32_t hash = 0;
  auto pipeLayoutSearchRes = m_activePipelineLayout.find(hash);

  if (pipeLayoutSearchRes == m_activePipelineLayout.end())
    return CreatePipelineLayout(hash);
  else
    return pipeLayoutSearchRes->second;
}

void PipelineStateManager::Init(Device& device)
{
  m_device = &device;
}

void PipelineStateManager::CleanUp()
{
  for (auto& pipeline : m_activePipelineLayout)
  {
    API_CALL(vkDestroyPipelineLayout, *m_device, pipeline.second.pipelineLayout, nullptr);
  }
  m_activePipelines.clear();

  for (auto& rp : m_activeRenderpass)
  {
    API_CALL(vkDestroyRenderPass, *m_device, rp.second.renderPass, nullptr);
  }
  m_activeRenderpass.clear();
}

Pipeline& PipelineStateManager::GetPipeline()
{
  // TODO: hash pipeline layouts and pipelines
  uint32_t layoutHash = 0;
  uint32_t renderPassHash = 0;

  // TODO: hash pipeline layouts and pipelines
  uint32_t hash = layoutHash + renderPassHash;
  // hash with layout hash
  auto pipe = m_activePipelines.find(hash);
  if(pipe != m_activePipelines.end())
    return (pipe->second);
  // pipeline does not exist at this point

  return CreatePipeline(hash);
}

RenderPass& PipelineStateManager::GetRenderpass()
{
  // TODO: hash render passes
  uint32_t hash = 0;
  auto rp = m_activeRenderpass.find(hash);
  if (rp != m_activeRenderpass.end())
    return (rp->second);

  return CreateRenderpass(hash);
}

void PipelineStateManager::SetVertexInputState(VertexInputState state)
{
  m_vertexState = state;
}

void PipelineStateManager::SetTopology(VkPrimitiveTopology topology)
{
  m_topology = topology;
}

void PipelineStateManager::SetPolygonMode(VkPolygonMode polygonMode)
{
  m_polygonMode = polygonMode;
}

void PipelineStateManager::SetRTBlendState(RenderTargetBlendStates blendState, size_t targetRT)
{
  m_rtBlendStates[targetRT] = blendState;
}

void PipelineStateManager::SetRTClearvalue(uint32_t rtIndex, glm::vec4 clearValue)
{
  UNUSED_PARAM(rtIndex);
  UNUSED_PARAM(clearValue);
}

void PipelineStateManager::SetRenderTarget(uint32_t rtIndex, ImageView image)
{
  m_RenderTargetImageView[rtIndex] = image;
}

void PipelineStateManager::ResetRenderTargets()
{
  for (int i = 0; i < 8; ++i)
  {
    m_RenderTargetImageView[i].reset();
    m_rtBlendStates[i].blendEnabled = false;
    m_clearValues[i].reset();
  }
}

void PipelineStateManager::SetShader(Shader& shader)
{
  switch (shader.GetShaderType())
  {
  case ShaderType::CS:
    m_pixelShader   = nullptr;
    m_vertexShader  = nullptr;
    m_computeShader = &shader;
    break;
  case ShaderType::PS:
    m_pixelShader   = &shader;
    m_computeShader = nullptr;
    break;
  case ShaderType::VS:
    m_vertexShader  = &shader;
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
