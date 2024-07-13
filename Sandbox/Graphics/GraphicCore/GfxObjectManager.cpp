#include "GraphicEngine.h"
#include "GLFW/glfw3.h"
#include "GraphicDefines.hpp"
#include "Includes/Defines.h"
#include "Engine/PlatformManager.h"
#include "../ShaderManagement/GfxShaderManager.h"
#include "GfxPipelineStateManager.h"

#include <vector>
#include "GfxObjectManager.h"

void GfxObjectManager::Init(GfxDevice& device, uint32_t maxFrames)
{
    m_buffer.resize(maxFrames);
    for (uint32_t i = 0; i < maxFrames; ++i)
    {
        m_buffer[i].CreateLayout(device);
        m_buffer[i].CreateBuffer(c_maxObjects * sizeof(GfxObject));
    }
}

void GfxObjectManager::CleanUp()
{
    for (uint32_t i = 0; i < m_buffer.size(); ++i)
    {
        m_buffer[i].CleanUp();
        m_buffer[i].CleanUpLayouts();
    }
}

ObjectID GfxObjectManager::AddObject(GfxObject obj)
{
    m_objectList.emplace_back(obj);
    return ObjectID(m_objectList.size() - 1);
}

void GfxObjectManager::UpdateObject(ObjectID objectID, GfxObject obj)
{
    m_objectList[objectID.id] = obj;
}

void GfxObjectManager::UpdateBuffers(uint32_t currentFrame)
{
    m_currentFrame = currentFrame;
    m_buffer[currentFrame].UpdateBuffer(m_objectList.data(), 0, m_objectList.size() * sizeof(m_objectList[0]));
}

GfxStructuredBuffer& GfxObjectManager::GetBuffer()
{
    return m_buffer[m_currentFrame];
}
