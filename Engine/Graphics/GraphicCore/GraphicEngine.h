#pragma once
#include "Includes/Defines.h"
#include "vulkan/vulkan.h"
#include "Device.h"
#include "Surface.h"

#include <vector>

class GraphicEngine
{
  DefaultSingleton(GraphicEngine);

  VkInstance m_vkInstance;

#ifndef NDEBUG
  
  VkDebugUtilsMessengerEXT m_debugMessenger;

  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

  const std::vector<const char*> validationLayers = {
      "VK_LAYER_KHRONOS_validation"
  };
  void InitValidation();
  void SetupDebugMessenger();
  bool m_layerExtInitialised = false;
#endif

  std::vector<const char*> m_EnabledExtensions;
  std::vector<const char*> m_EnabledLayers;

  std::vector<VkExtensionProperties> m_availExtensions;
  std::vector<VkLayerProperties>     m_availLayers;
  VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

  Device m_device;
  Surface m_surface;
public:
  void InitLayerExtInfo();
  bool IsLayerSupported(const char* layerName);
  bool IsExtensionSupported(const char* extensionName);
  void AddLayer(const char* layerName, bool required = false);
  void AddExtension(const char* extensionName, bool required = false);

  void Init();

  void Cleanup();
};