#include "GraphicEngine.h"
#include "GLFW/glfw3.h"
#include "GraphicDefines.hpp"
#include "Includes/Defines.h"
#include "Engine/PlatformManager.h"
#include "../ShaderManagement/ShaderManager.h"

#include <vector>

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

GraphicEngine::GraphicEngine() : 
  m_requiredVulkanExtensions{
    },
  m_requiredLayers{
    },
  m_optionalVulkanExtensions{
    },
  m_optionalLayers{
    },
  m_requiredDeviceExtensions{
     VK_KHR_SWAPCHAIN_EXTENSION_NAME
    },
  m_optionalDeviceExtensions{
    }
{
#ifdef DEBUG
  m_requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
  m_requiredVulkanExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
}

VKAPI_ATTR VkBool32 VKAPI_CALL GraphicEngine::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
  UNUSED_PARAM(messageType);
  UNUSED_PARAM(pUserData);

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
    if(strcmp(itr.layerName, layerName) == 0)
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
  if(IsLayerSupported(layerName))
    m_EnabledLayers.push_back(layerName);
  else
  {
    assert(!required);
    Log("Layer %s not avaiable", Severe, layerName);
  }
}

void GraphicEngine::AddExtension(const char* extensionName, bool required)
{
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

  m_EnabledExtensions.insert(m_EnabledExtensions.end(),glfwExtensions, glfwExtensions + glfwExtensionCount);

  for (const auto name : m_requiredVulkanExtensions)
    AddExtension(name, true);
  for (const auto name : m_requiredLayers)
    AddLayer(name, true);
  for (const auto name : m_optionalVulkanExtensions)
    AddExtension(name);
  for (const auto name : m_optionalLayers)
    AddLayer(name);

  createInfo.enabledLayerCount       = static_cast<uint32_t>(m_EnabledLayers.size());
  createInfo.enabledExtensionCount   = static_cast<uint32_t>(m_EnabledExtensions.size());
  createInfo.ppEnabledLayerNames     = m_EnabledLayers.data();
  createInfo.ppEnabledExtensionNames = m_EnabledExtensions.data();
  
  API_CALL(&vkCreateInstance, &createInfo, nullptr, &m_vkInstance);

#ifndef NDEBUG
  SetupDebugMessenger();
#endif
  m_surface.Init(m_vkInstance);
  m_device.RegisterExtensions(m_requiredDeviceExtensions, m_optionalDeviceExtensions);
  m_device.Init(m_vkInstance, m_surface);
  {
    VkSurfaceFormatKHR swapchainFormat = { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    m_swapChain.Init( m_device, swapchainFormat, m_device.GetQueueFamily(m_device, m_surface), m_surface, PlatformManager::GetInstance().GetSize());
  }
  ShaderManager::CreateInstance();
  ShaderManager::GetInstance().Init(m_device);
}

void GraphicEngine::Cleanup()
{
#ifndef NDEBUG
    API_CALL(DestroyDebugUtilsMessengerEXT, m_vkInstance, m_debugMessenger, nullptr);
#endif
    m_swapChain.Cleanup();
    m_device.CleanUp();
    m_surface.Cleanup();

    API_CALL(vkDestroyInstance, m_vkInstance, nullptr);
}

