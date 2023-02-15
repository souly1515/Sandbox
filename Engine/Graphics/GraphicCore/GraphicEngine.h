#pragma once
#include "Includes/Defines.h"
#include "vulkan/vulkan.h"
#include "Device.h"
#include "Surface.h"
#include "Swapchain.h"

#include <vector>

class GraphicEngine
{
  DefaultSingleton(GraphicEngine);
private:
  GraphicEngine();
  VkInstance m_vkInstance = VK_NULL_HANDLE;

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

  Device m_device;
  Surface m_surface;
  Swapchain m_swapChain;
public:
  void InitLayerExtInfo();
  bool IsLayerSupported(const char* layerName);
  bool IsExtensionSupported(const char* extensionName);
  void AddLayer(const char* layerName, bool required = false);
  void AddExtension(const char* extensionName, bool required = false);

  void Init();

  void Cleanup();
};