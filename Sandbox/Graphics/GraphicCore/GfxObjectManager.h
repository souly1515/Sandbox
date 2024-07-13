#pragma once
#include "Includes/Defines.h"
#include "vulkan/vulkan.h"
#include "GfxDevice.h"
#include "GfxStructuredBuffer.h"
#include "GfxObject.h"

class GfxObjectManager
{
    DefaultSingleton(GfxObjectManager);
private:
    GfxObjectManager() = default;
    std::vector<GfxStructuredBuffer> m_buffer;
    std::vector<GfxObject> m_objectList;
    // sorted dirtyList (by size) for updated objects to move in
    // when updating just copying either the back or a large enough empty space
    std::vector<uint32_t> m_indexList; // eventually this will be used to track the elements pos in the object list
    const uint32_t c_maxObjects = 256;
    uint32_t m_currentFrame;
public:
    void Init(GfxDevice& device, uint32_t maxFrames);

    void CleanUp();

    // return handle that will update position if the object is modified?
    ObjectID AddObject(GfxObject obj = {});

    void UpdateObject(ObjectID objectID, GfxObject obj);
    void UpdateBuffers(uint32_t currentFrame);

    GfxStructuredBuffer& GetBuffer();
};