#include "GraphicEngine.h"
#include "GLFW/glfw3.h"
#include "GraphicDefines.hpp"
#include "Includes/Defines.h"

#include <vector>

void GraphicEngine::InitValidation(std::vector<const char*>& enabledLayers, std::vector<const char*>& enabledExtensions)
{
  for (const char* layerName : validationLayers) {
    bool layerFound = false;

    for (const auto& layerProperties : m_layers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      Log("validation layer %s not found", MessageSeverity::Severe, layerName);
      return;
    }
  }

  for (const char* layerName : validationLayers) {
    enabledLayers.push_back(layerName);
  }
  enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
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

  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

  m_extensions.resize(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, m_extensions.data());

  Log("available extensions:\n", Verbose);

  for (const auto& extension : m_extensions)
  {
    Log("\t%s\n", Verbose, extension.extensionName);
  }

  uint32_t layerCount = 0;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
  m_layers.resize(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, m_layers.data());

  Log("available Layers:\n", Verbose);
  Log("layer count %d", Verbose, layerCount);
  for (const auto& layer : m_layers)
  {
    Log("\t%s\n", Verbose, layer.layerName);
  }

  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char*> enabledLayers;
  std::vector<const char*> enabledExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifndef NDEBUG
  InitValidation(enabledLayers, enabledExtensions);
#endif

  createInfo.enabledLayerCount       = static_cast<uint32_t>(enabledLayers.size());
  createInfo.enabledExtensionCount   = static_cast<uint32_t>(enabledExtensions.size());
  createInfo.ppEnabledLayerNames     = enabledLayers.data();
  createInfo.ppEnabledExtensionNames = enabledExtensions.data();
  
  API_CALL(&vkCreateInstance, &createInfo, nullptr, &instance);
}

