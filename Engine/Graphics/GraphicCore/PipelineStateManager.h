#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"

#include "Includes/Defines.h"
#include "GraphicDefines.hpp"
#include "Device.h"
#include "ImageView.h"

#include "Graphics/ShaderManagement/Shader.h"

#include "glm/glm.hpp"
#include <unordered_map>

struct PipelineLayout
{
  VkPipelineLayout pipelineLayout;
};

struct Pipeline
{
  VkPipeline pipeline;
};

struct RenderPass
{
  VkRenderPass renderPass;
};

struct FrameBuffer
{
  VkFramebuffer frameBuffer;
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

class PipelineStateManager
{
  DefaultSingleton(PipelineStateManager);
private:
  std::unordered_map<uint64_t, Pipeline> m_activePipelines;
  std::unordered_map<uint64_t, RenderPass> m_activeRenderpass;
  std::unordered_map<uint64_t, FrameBuffer> m_activeFrameBuffer;
  std::unordered_map<uint64_t, PipelineLayout> m_activePipelineLayout;
  std::optional<ImageView> m_RenderTargetImageView[8];

  Device* m_device;
  // Pipeline layout variables
  VertexInputState m_vertexState;
  VkPrimitiveTopology m_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  VkPolygonMode m_polygonMode = VK_POLYGON_MODE_FILL;

  RenderTargetBlendStates m_rtBlendStates[8];

  // renderpass variables
  std::optional<glm::vec4> m_clearValues[8];

  Pipeline&       CreatePipeline       (uint32_t hash);
  RenderPass&     CreateRenderpass     (uint32_t hash);
  FrameBuffer&    CreateFramebuffer    (uint32_t hash);
  PipelineLayout& CreatePipelineLayout (uint32_t hash);

  void FillPipelineCreateMiscInfo(VkGraphicsPipelineCreateInfo& pipelineInfo);

  PipelineLayout& GetPipelineLayout();
  

  uint32_t shaderCount = 0;
  Shader* m_computeShader = nullptr;
  Shader* m_pixelShader   = nullptr;
  Shader* m_vertexShader  = nullptr;
public:
  void Init(Device& device);
  void CleanUp();

  Pipeline& GetPipeline();
  RenderPass& GetRenderpass();
  FrameBuffer& GetFrameBuffer();

  void SetVertexInputState(VertexInputState state);
  void SetTopology(VkPrimitiveTopology topology);
  void SetPolygonMode(VkPolygonMode polygonMode);
  void SetRTBlendState(RenderTargetBlendStates blendState, size_t targetRT);

  void SetRTClearvalue(uint32_t rtIndex, glm::vec4 clearValue);
  void SetRenderTarget(uint32_t rtIndex, ImageView image);
  void ResetRenderTargets();

  void SetShader(Shader& shader);
};

