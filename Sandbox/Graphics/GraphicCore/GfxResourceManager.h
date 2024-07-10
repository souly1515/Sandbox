#pragma once
#include "Includes/Defines.h"
#include "vulkan/vulkan.h"
#include "Engine/UID.h"
#include "GfxImageView.h"
#include "GfxDevice.h"

#include <unordered_map>

class GfxResourceManager
{
    DefaultSingleton(GfxResourceManager);
private:
    GfxResourceManager() = default;
    std::unordered_map<uint32_t, GfxImageView> m_IVMap;
    GfxDevice* m_device;
public:
    // TODO: handle initialization of resource object
    static void NewImageView(uint32_t uid) 
    {
        ms_instance->m_IVMap.emplace(uid, GfxImageView{});
    };
    static void NewRenderTarget(uint32_t uid, glm::vec2 extent, VkFormat format)
    {
        // TODO
        UNUSED_PARAM(format);
        UNUSED_PARAM(extent);
        GfxImage image;
        // create image
        // create image view
        // place image view into map
        GfxImageView temp{};
        //temp.Init();
        ms_instance->m_IVMap.emplace(uid, temp);
    };

    void Init(GfxDevice& device)
    {
        m_device = &device;
    }

    static GfxImageView& GetImageView(uint32_t uid) { return ms_instance->m_IVMap.at(uid); };

    static void CleanUpFrame() { ms_instance->m_IVMap.clear(); };

};