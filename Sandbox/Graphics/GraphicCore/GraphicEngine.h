#pragma once
#include "Includes/Defines.h"
#include "vulkan/vulkan.h"
#include "GfxDevice.h"
#include "GfxSurface.h"
#include "GfxPipelineStateManager.h"
#include "GfxCommandPool.h"

#include <vector>

class GraphicEngine
{
    DefaultSingleton(GraphicEngine);
private:

    class FrameSyncer
    {
    };

    GraphicEngine();
    VkInstance m_vkInstance = VK_NULL_HANDLE;

    const int MAX_FRAMES_IN_FLIGHT = 2;

#ifndef NDEBUG

    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    void SetupDebugMessenger();
    bool m_layerExtInitialised = false;
#endif

    std::vector<const char*> m_requiredVulkanExtensions;
    std::vector<const char*> m_requiredLayers;
    std::vector<const char*> m_optionalVulkanExtensions;
    std::vector<const char*> m_optionalLayers;
    std::vector<const char*> m_requiredDeviceExtensions;
    std::vector<const char*> m_optionalDeviceExtensions;

    std::vector<const char*> m_EnabledExtensions;
    std::vector<const char*> m_EnabledLayers;

    std::vector<VkExtensionProperties> m_availExtensions;
    std::vector<VkLayerProperties>     m_availLayers;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

    GfxDevice m_device;

    GfxCommandPool m_commandPool;

    GfxCommandBuffer m_currentCommmandBuffer;

    GfxPipelineStateManager* m_cachedPipelineManager;

    std::vector<VkSemaphore> imageAvailableSemaphore;
    std::vector<VkSemaphore> renderFinishedSemaphore;
    std::vector<VkFence> inFlightFence;

    uint32_t m_currentFrameIndex;

    void InitSyncObjects();
    void CleanupSyncObjects();
public:
    void InitLayerExtInfo();
    bool IsLayerSupported(const char* layerName);
    bool IsExtensionSupported(const char* extensionName);
    void AddLayer(const char* layerName, bool required = false);
    void AddExtension(const char* extensionName, bool required = false);
    GfxDevice& GetDevice() { return m_device; };

    void Init();
    void Cleanup();

    void BeginRenderPass();
    void EndRenderPass();

    void CommitStates();

    void BeginOutOfFrameRecording();
    void EndOutOfFrameRecording();

    void BeginRecordingGraphics();
    void BeginRecordingCompute();
    void EndRecording();

    void StartFrame();
    void Submit();
    void SubmitWithSync();
    void Flip();

    const GfxCommandBuffer& GetCurrentCommandBuffer();
};