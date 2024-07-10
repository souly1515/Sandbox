#include "Engine.h"

#include "PlatformManager.h"
#include "Graphics/GraphicCore/GraphicEngine.h"
#include "Graphics/GraphicCore/GfxPipelineStateManager.h"
#include "Graphics/ShaderManagement/GfxShaderManager.h"
#include "shaderData.h"
#include "Graphics/GraphicCore/GfxResourceManager.h"
#include "Graphics/GraphicCore/GfxBuffer.h"
#include "Graphics/GraphicCore/GfxVertex.h"
#include <iostream>

DeclareIV(RT0);

int Engine::MainLoop()
{
    PlatformManager& pm = PlatformManager::GetInstance();
    GraphicEngine& ge = GraphicEngine::GetInstance();

    const std::vector<GfxStandardVertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    }; 

    const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
    };

    size_t bufferSize = sizeof(vertices[0]) * vertices.size();
    size_t indexBufferSize = sizeof(indices[0]) * indices.size();

    GfxBuffer stagingBuffer;
    stagingBuffer.CreateBuffer(ge.GetDevice(), bufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    GfxBuffer vertexBuffer;
    vertexBuffer.CreateBuffer(ge.GetDevice(), bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    GfxBuffer indexBuffer;
    indexBuffer.CreateBuffer(ge.GetDevice(), indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    {
        void* stagingMem = stagingBuffer.Map();
        memcpy(stagingMem, vertices.data(), vertexBuffer.GetSize());
        stagingBuffer.Unmap();

        ge.BeginOutOfFrameRecording();
        ge.BeginRecordingGraphics();
        stagingBuffer.CopyTo(vertexBuffer, ge.GetCurrentCommandBuffer(), bufferSize);

        ge.EndRecording();

        ge.Submit();
    }

    {
        void* stagingMem = stagingBuffer.Map(indexBufferSize, bufferSize);
        memcpy(stagingMem, indices.data(), indexBuffer.GetSize());
        stagingBuffer.Unmap();

        ge.BeginRecordingGraphics();
        stagingBuffer.CopyTo(indexBuffer, ge.GetCurrentCommandBuffer(), indexBufferSize, bufferSize);

        ge.EndRecording();

        ge.Submit();
    }



    ge.EndOutOfFrameRecording();


    while (!pm.CheckExit())
    {
        pm.HandleIO();

        ge.StartFrame();

        GfxPipelineStateManager::GetInstance().SetShader(GfxShaderManager::GetShader(VS_BasicShader::Hash));
        GfxPipelineStateManager::GetInstance().SetShader(GfxShaderManager::GetShader(PS_BasicShader::Hash));
        GfxPipelineStateManager::GetInstance().SetRenderTarget(0, ge.GetDevice().GetSwapChain().GetCurrentImageView());

        ge.CommitStates();

        ge.BeginRenderPass();

        VkBuffer vertexBuffers[] = { vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(ge.GetCurrentCommandBuffer(), 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(ge.GetCurrentCommandBuffer(), indexBuffer, 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(ge.GetCurrentCommandBuffer(), static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

        ge.EndRenderPass();

        ge.EndRecording();

        ge.SubmitWithSync();
        ge.Flip();

        GfxResourceManager::CleanUpFrame();
    }
    vkDeviceWaitIdle(ge.GetDevice());
    vertexBuffer.CleanUp();
    stagingBuffer.CleanUp();
    indexBuffer.CleanUp();
    CleanUp();

    return m_returnValue;
}

void Engine::PrintShit()
{
    std::cout << "printing shit" << std::endl;
}

void Engine::Init()
{
    PlatformManager::CreateInstance();
    GraphicEngine::CreateInstance();

    PlatformManager::GetInstance().Init();

    GraphicEngine::GetInstance().InitLayerExtInfo();

    // probably add all additional layers and extensions here

    GraphicEngine::GetInstance().Init();
}

void Engine::CleanUp()
{
    GraphicEngine::GetInstance().Cleanup();
    PlatformManager::GetInstance().CleanUp();
}



