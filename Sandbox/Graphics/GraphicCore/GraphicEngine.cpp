#include "GraphicEngine.h"
#include "GLFW/glfw3.h"
#include "GraphicDefines.hpp"
#include "Includes/Defines.h"
#include "Engine/PlatformManager.h"
#include "../ShaderManagement/GfxShaderManager.h"

#include <vector>

uint32_t thread_local GraphicEngine::ms_thread_id = 0;

void* operator new(std::size_t count)
{
    auto ptr = malloc(count);
    TracyAlloc(ptr, count);
    return ptr;
}

void operator delete(void* ptr) noexcept
{
    TracyFree(ptr);
    free(ptr);
}


GraphicEngine::GraphicEngine() :
    m_requiredVulkanExtensions{},
    m_requiredLayers{},
    m_optionalVulkanExtensions{},
    m_optionalLayers{},
    m_requiredDeviceExtensions{
       VK_KHR_SWAPCHAIN_EXTENSION_NAME
    },
    m_optionalDeviceExtensions{},
    m_currentCommmandBuffer{ }
{
#ifdef DEBUG
    m_requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
    m_requiredVulkanExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
}

#ifndef NDEBUG
// probably offload this to some sort of extension manager at some point
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL GraphicEngine::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    UNUSED_PARAM(messageType);
    UNUSED_PARAM(pUserData);

    const bool c_AllValidations = false;

    if constexpr (!c_AllValidations)
    {
        const int DisabledValidationErrors[] =
        {
            -1070202321 // VUID-VkSwapchainCreateInfoKHR-imageFormat-01778 Rivatune
            , 1636479463//, "VUID-VkImageViewCreateInfo-usage-02275" Rivatune
        };

        for (uint32_t i = 0; i < sizeof(DisabledValidationErrors); ++i)
        {
            if (pCallbackData->messageIdNumber == DisabledValidationErrors[i])
            {
                return VK_FALSE;
            }
        }
    }

    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        return VK_FALSE;
        // TMI
        //  Log("Vulkan Verbose: %s\n", Debug, pCallbackData->pMessage);
        //  break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        Log("Vulkan Info : %s\n", Debug, pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        Log("Vulkan Warning: %s\n", Debug, pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        Log("Vulkan Error: %s\n", Debug, pCallbackData->pMessage);
        break;
    default:
        Log("Vulkan Unkown: %s\n", Debug, pCallbackData->pMessage);
        break;
    }
    __debugbreak();
    return VK_FALSE;
}

void GraphicEngine::SetupDebugMessenger()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // Optional

    API_CALL(CreateDebugUtilsMessengerEXT, m_vkInstance, &createInfo, nullptr, &m_debugMessenger);
}
#endif

void GraphicEngine::InitSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    imageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFence.resize(MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        API_CALL(vkCreateSemaphore, m_device, &semaphoreInfo, nullptr, &imageAvailableSemaphore[i]);
        API_CALL(vkCreateSemaphore, m_device, &semaphoreInfo, nullptr, &renderFinishedSemaphore[i]);
        API_CALL(vkCreateFence, m_device, &fenceInfo, nullptr, &inFlightFence[i]);
    }
}

void GraphicEngine::CleanupSyncObjects()
{
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        API_CALL(vkDestroySemaphore, m_device, imageAvailableSemaphore[i], nullptr);
        API_CALL(vkDestroySemaphore, m_device, renderFinishedSemaphore[i], nullptr);
        API_CALL(vkDestroyFence, m_device, inFlightFence[i], nullptr);
    }
}

void GraphicEngine::InitLayerExtInfo()
{
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        m_availExtensions.resize(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, m_availExtensions.data());
    }
    {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        m_availLayers.resize(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, m_availLayers.data());
    }
#ifdef DEBUG
    Log("available extensions:\n", Verbose);

    for (const auto& extension : m_availExtensions)
    {
        Log("\t%s\n", Verbose, extension.extensionName);
    }

    Log("available Layers:\n", Verbose);
    for (const auto& layer : m_availLayers)
    {
        Log("\t%s\n", Verbose, layer.layerName);
    }
#endif
    DEBUG_ONLY(m_layerExtInitialised = true);
}

bool GraphicEngine::IsLayerSupported(const char* layerName)
{
    assert(m_layerExtInitialised);
    for (const auto& itr : m_availLayers)
    {
        if (strcmp(itr.layerName, layerName) == 0)
            return true;
    }
    return false;
}

bool GraphicEngine::IsExtensionSupported(const char* extensionName)
{
    assert(m_layerExtInitialised);
    for (const auto& itr : m_availExtensions)
    {
        if (strcmp(itr.extensionName, extensionName) == 0)
            return true;
    }
    return false;
}

void GraphicEngine::AddLayer(const char* layerName, bool required)
{
    UNUSED_PARAM(required);
    if (IsLayerSupported(layerName))
        m_EnabledLayers.push_back(layerName);
    else
    {
        assert(!required);
        Log("Layer %s not avaiable", Severe, layerName);
    }
}

void GraphicEngine::AddExtension(const char* extensionName, bool required)
{
    UNUSED_PARAM(required);
    if (IsExtensionSupported(extensionName))
        m_EnabledExtensions.push_back(extensionName);
    else
    {
        assert(!required);
        Log("Extension %s not avaiable", Severe, extensionName);
    }
}

void GraphicEngine::Init()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    m_EnabledExtensions.insert(m_EnabledExtensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);

    for (const auto name : m_requiredVulkanExtensions)
        AddExtension(name, true);
    for (const auto name : m_requiredLayers)
        AddLayer(name, true);
    for (const auto name : m_optionalVulkanExtensions)
        AddExtension(name);
    for (const auto name : m_optionalLayers)
        AddLayer(name);

    createInfo.enabledLayerCount = static_cast<uint32_t>(m_EnabledLayers.size());
    createInfo.enabledExtensionCount = static_cast<uint32_t>(m_EnabledExtensions.size());
    createInfo.ppEnabledLayerNames = m_EnabledLayers.data();
    createInfo.ppEnabledExtensionNames = m_EnabledExtensions.data();

    API_CALL(&vkCreateInstance, &createInfo, nullptr, &m_vkInstance);

#ifndef NDEBUG

    if (enableValidationLayers)
    {
        SetupDebugMessenger();
    }
#endif
    m_device.RegisterExtensions(m_requiredDeviceExtensions, m_optionalDeviceExtensions);
    m_device.Init(m_vkInstance);
    GfxShaderManager::CreateInstance();
    GfxShaderManager::GetInstance().Init(m_device);
    GfxPipelineStateManager::CreateInstance();
    m_cachedPipelineManager = GfxPipelineStateManager::GetInstancePtr();
    m_cachedPipelineManager->Init(m_device);

    m_commandPool.Init(m_device);

    // set up default RT blend state
    {
        RenderTargetBlendStates blendState;
        blendState.blendEnabled = true;
        blendState.srcColorBlend = VK_BLEND_FACTOR_SRC_ALPHA;
        blendState.dstColorBlend = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendState.colorBlendOp = VK_BLEND_OP_ADD;
        blendState.srcAlphaBlend = VK_BLEND_FACTOR_ONE;
        blendState.dstAlphaBlend = VK_BLEND_FACTOR_ZERO;
        blendState.alphaBlendOp = VK_BLEND_OP_ADD;
        m_cachedPipelineManager->SetRTBlendState(blendState, 0);
    }
    GfxResourceManager::CreateInstance();

    GfxDescriptorPool::CreateInstance();

    GfxObjectManager::CreateInstance();

    GfxDescriptorPool::GetInstance().InitPools(m_device);

    m_objectManager = GfxObjectManager::GetInstancePtr();
    m_objectManager->Init(m_device, MAX_FRAMES_IN_FLIGHT);


    InitSyncObjects();

#ifdef PROFILE
    m_profileCommandBuffer = m_commandPool.GetUntrackedCommandBuffer();
    m_tracyContext = TracyVkContext(m_device, m_device, m_device.GetGraphicsQueue(), m_profileCommandBuffer);
#endif
}

void GraphicEngine::BeginRenderPass()
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

    GfxRenderState renderState = m_cachedPipelineManager->GetRenderState();
    renderPassInfo.renderPass = renderState.renderPass;
    renderPassInfo.framebuffer = renderState.frameBuffer;

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_device.GetSwapChain().GetVkExtent();

    VkClearValue clearColor[8];
    uint32_t i = 0;
    while (renderState.clearValues[i].has_value())
    {
        clearColor[i] = { renderState.clearValues[i]->x, renderState.clearValues[i]->y, renderState.clearValues[i]->z, renderState.clearValues[i]->z };
        ++i;
    }
    renderPassInfo.pClearValues = clearColor;
    renderPassInfo.clearValueCount = i;

    API_CALL(vkCmdBeginRenderPass, m_currentCommmandBuffer[ms_thread_id], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

}

void GraphicEngine::EndRenderPass()
{
    API_CALL(vkCmdEndRenderPass, m_currentCommmandBuffer[ms_thread_id]);
}

void GraphicEngine::CommitStates()
{
    m_cachedPipelineManager->CommitStates(m_currentCommmandBuffer[ms_thread_id]);

    VkExtent2D swapChainExtent = m_device.GetSwapChain().GetVkExtent();
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_currentCommmandBuffer[ms_thread_id], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(m_currentCommmandBuffer[ms_thread_id], 0, 1, &scissor);
}

void GraphicEngine::BeginOutOfFrameRecording()
{
    m_commandPool.SetCurrentFrame(MAX_FRAMES_IN_FLIGHT + 1);
}

void GraphicEngine::EndOutOfFrameRecording()
{
    m_commandPool.FrameFlip();
}

void GraphicEngine::BeginRecordingGraphics()
{
    // expected to close existing command list before recording 
    assert(!m_currentCommmandBuffer[ms_thread_id]);
    assert(!m_currentCommmandBuffer[ms_thread_id].IsOpen());

    m_currentCommmandBuffer[ms_thread_id] = m_commandPool.GetGraphicsCommandBuffer();

    m_currentCommmandBuffer[ms_thread_id].StartRecording();
}

void GraphicEngine::BeginRecordingCompute()
{
    // expected to close existing command list before recording 
    assert(!m_currentCommmandBuffer[ms_thread_id]);
    assert(m_currentCommmandBuffer[ms_thread_id].IsOpen());

    m_currentCommmandBuffer[ms_thread_id] = m_commandPool.GetComputeCommandBuffer();

    m_currentCommmandBuffer[ms_thread_id].StartRecording();
}

void GraphicEngine::EndRecording()
{
    m_currentCommmandBuffer[ms_thread_id].EndRecording();
}

void GraphicEngine::StartFrame()
{
    CPU_ProfileZone(StartFrame);
    uint32_t imageIndex = 0;

    m_currentFrameIndex = (m_currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

    vkWaitForFences(m_device, 1, &inFlightFence[m_currentFrameIndex], VK_TRUE, UINT64_MAX);
    vkResetFences(m_device, 1, &inFlightFence[m_currentFrameIndex]);
    vkAcquireNextImageKHR(m_device, m_device.GetSwapChain(), UINT64_MAX, imageAvailableSemaphore[m_currentFrameIndex], VK_NULL_HANDLE, &imageIndex);
    m_device.GetSwapChain().SetCurrentImageIndex(imageIndex);
    m_commandPool.SetCurrentFrame(m_currentFrameIndex);

}

void GraphicEngine::Submit()
{
    const auto& commandBuffers = m_commandPool.GetCurrentGraphicsCommandBuffers();
    if (commandBuffers.size())
    {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        submitInfo.commandBufferCount = (uint32_t)commandBuffers.size();
        submitInfo.pCommandBuffers = commandBuffers.data();

        vkQueueSubmit(m_device.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        m_commandPool.SubmitGraphics();
    }
}

void GraphicEngine::SubmitWithSync()
{
    CPU_ProfileZone(Submit)
    const auto& commandBuffers = m_commandPool.GetCurrentGraphicsCommandBuffers();
    if (commandBuffers.size())
    {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphore[m_currentFrameIndex] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;


        submitInfo.commandBufferCount = (uint32_t)commandBuffers.size();
        submitInfo.pCommandBuffers = commandBuffers.data();

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphore[m_currentFrameIndex] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkQueueSubmit(m_device.GetGraphicsQueue(), 1, &submitInfo, inFlightFence[m_currentFrameIndex]);
        m_commandPool.SubmitGraphics();
    }
}

void GraphicEngine::Flip()
{
    CPU_ProfileZone(Flip)
    VkSemaphore signalSemaphores[] = { renderFinishedSemaphore[m_currentFrameIndex] };

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { m_device.GetSwapChain() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &m_device.GetSwapChain().GetCurrentImageIndex();
    presentInfo.pResults = nullptr; // Optional

    vkQueuePresentKHR(m_device.GetPresentQueue(), &presentInfo);

    m_commandPool.FrameFlip();
}

uint32_t GraphicEngine::GetCurrentFrame()
{
    return m_currentFrameIndex;
}

const GfxCommandBuffer& GraphicEngine::GetCurrentCommandBuffer()
{
    assert(m_currentCommmandBuffer);
    return m_currentCommmandBuffer[ms_thread_id];
}

#ifdef PROFILE
GfxCommandBuffer& GraphicEngine::GetProfileCommandBuffer()
{
    return m_profileCommandBuffer;
}

tracy::VkCtx* GraphicEngine::GetProfileContext()
{
    return m_tracyContext;
}
#endif

void GraphicEngine::Cleanup()
{
    vkDeviceWaitIdle(m_device);
    CleanupSyncObjects();

#ifdef PROFILE
    m_commandPool.ReleaseUntrackedCommandBuffer(m_profileCommandBuffer.GetVkCommandBuffer());
    TracyVkDestroy(m_tracyContext);
#endif

    m_objectManager->CleanUp();
    GfxDescriptorPool::GetInstance().CleanUp();
    m_commandPool.CleanUp();
    GfxShaderManager::GetInstance().CleanUp();
    GfxPipelineStateManager::GetInstance().CleanUp();

    m_device.CleanUp();

#ifndef NDEBUG
    API_CALL(DestroyDebugUtilsMessengerEXT, m_vkInstance, m_debugMessenger, nullptr);
#endif

    API_CALL(vkDestroyInstance, m_vkInstance, nullptr);
}

