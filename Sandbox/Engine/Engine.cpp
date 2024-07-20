#include "Engine.h"

#include "PlatformManager.h"
#include "Graphics/GraphicCore/GraphicEngine.h"
#include "Graphics/GraphicCore/GfxPipelineStateManager.h"
#include "Graphics/ShaderManagement/GfxShaderManager.h"
#include "shaderData.h"
#include "Graphics/GraphicCore/GfxResourceManager.h"
#include "Graphics/GraphicCore/GfxObjectManager.h"

// eventually remove
#include "Graphics/GraphicCore/GfxBuffer.h"
#include "Graphics/GraphicCore/GfxVertex.h"
#include "Graphics/GraphicCore/GfxUniformBuffer.hpp"
// end

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <chrono>
#include <tracy/public/common/TracySystem.hpp>

DeclareIV(RT0);

//DefineProfileMarker(SingleDraw)
const char* SingleDraw_ProfileMarker = "SingleDraw";

struct UniformBufferObject {
    glm::mat4 view;
    glm::mat4 proj;
};

void UpdateMatrices(GfxUniformBuffer<UniformBufferObject>& ubo, ObjectID* objID)
{
    GraphicEngine& ge = GraphicEngine::GetInstance();
    static auto startTime = std::chrono::high_resolution_clock::now();
    GfxObject obj;
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    obj.globalTransform = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    GfxObjectManager::GetInstance().UpdateObject(objID[0], obj);

    obj.globalTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0, 1, 0));
    obj.globalTransform = glm::rotate(obj.globalTransform, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, -1.0f));

    GfxObjectManager::GetInstance().UpdateObject(objID[1], obj);

    ubo.m_data.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.m_data.proj = glm::perspective(glm::radians(45.0f), ge.GetDevice().GetSwapChain().GetExtent().x / (float)ge.GetDevice().GetSwapChain().GetExtent().y, 0.1f, 10.0f);
    ubo.m_data.proj[1][1] *= -1;
    ubo.UpdateBuffer();
}

int Engine::MainLoop()
{
    PlatformManager& pm = PlatformManager::GetInstance();
    GraphicEngine& ge = GraphicEngine::GetInstance();
    GfxPipelineStateManager& psm = GfxPipelineStateManager::GetInstance();
    GfxObjectManager& om = GfxObjectManager::GetInstance();

    // TODO BLOCK - temp code to make things work first
    // temp ubo code
    std::vector<GfxUniformBuffer<UniformBufferObject>> uboTest;
    uboTest.resize(ge.MAX_FRAMES_IN_FLIGHT);

    GfxUniformBuffer<UniformBufferObject>::CreateLayout(ge.GetDevice());

    for (size_t i = 0; i < ge.MAX_FRAMES_IN_FLIGHT; ++i)
    {
        uboTest[i].CreateBuffer();
    }

    // temp index + vertex buffer code
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

    // temp object manager code
    ObjectID objectHandle[2];
    objectHandle[0] = om.AddObject();
    objectHandle[1] = om.AddObject();
    // TODO END


    while (!pm.CheckExit())
    {
        pm.HandleIO();

        ge.StartFrame();
        {
            CPU_ProfileZone(BufferUpdate);
            UpdateMatrices(uboTest[ge.GetCurrentFrame()], objectHandle);
            om.UpdateBuffers(ge.GetCurrentFrame());
        }

        ge.BeginRecordingGraphics();
        {
            CPU_ProfileZone(Recording_Command_Buffer);
            GPU_ProfileZone(SingleDraw);

            psm.SetRenderTarget(0, ge.GetDevice().GetSwapChain().GetCurrentImageView());
            ge.BeginRenderPass();

            psm.SetShader(GfxShaderManager::GetShader(VS_BasicShader::Hash));
            psm.SetShader(GfxShaderManager::GetShader(PS_BasicShader::Hash));

            // TODO BLOCK - below should be encompassed into the draw func
            psm.BindDescriptor(uboTest[ge.GetCurrentFrame()]);
            psm.BindStructuredBuffer(om.GetBuffer());
            ge.CommitStates();

            // move the following into pipeline state manager as well
            VkBuffer vertexBuffers[] = { vertexBuffer };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(ge.GetCurrentCommandBuffer(), 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(ge.GetCurrentCommandBuffer(), indexBuffer, 0, VK_INDEX_TYPE_UINT16);

            vkCmdDrawIndexed(ge.GetCurrentCommandBuffer(), static_cast<uint32_t>(indices.size()), 2, 0, 0, 0);

            //TODO END

            ge.EndRenderPass();
        }
        TracyVkCollect(ge.GetProfileContext(), ge.GetCurrentCommandBuffer());
        ge.EndRecording();

        ge.SubmitWithSync();
        ge.Flip();

        GfxResourceManager::CleanUpFrame();

        FrameMark;
    }
    vkDeviceWaitIdle(ge.GetDevice());

    for (int i = 0; i < ge.MAX_FRAMES_IN_FLIGHT; ++i)
    {
        uboTest[i].CleanUp();
    }

    GfxUniformBuffer<UniformBufferObject>::CleanUpLayouts();

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
    tracy::SetThreadName("MainThread");
}

void Engine::CleanUp()
{
    GraphicEngine::GetInstance().Cleanup();
    PlatformManager::GetInstance().CleanUp();
}



