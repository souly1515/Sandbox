#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"

#include "Includes/Defines.h"
#include "GraphicDefines.hpp"
#include "GfxDevice.h"

class GfxDescriptorPool
{
    DefaultSingleton(GfxDescriptorPool);

    VkDescriptorPool m_uniformPool;
    VkDescriptorPool m_storagePool;
    GfxDevice* m_device;
public:
    VkDescriptorPool GetUniformPool();
    VkDescriptorPool GetStoragePool();
    void InitPools(GfxDevice& device);
    void CleanUp();
};
