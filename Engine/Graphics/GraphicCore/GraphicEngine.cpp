#include "GraphicEngine.h"
#include "GLFW/glfw3.h"
#include "GraphicDefines.hpp"
#include "Includes/Defines.h"

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

VKAPI_ATTR VkBool32 VKAPI_CALL GraphicEngine::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
  UNUSED_PARAM(messageSeverity);
  UNUSED_PARAM(messageType);
  UNUSED_PARAM(pUserData);
  Log("validation layer: %s\n", Debug,pCallbackData->pMessage);

  return VK_FALSE;
}
void GraphicEngine::InitValidation()
{
  AddLayer("VK_LAYER_KHRONOS_validation", true);
  AddExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, true);
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

#ifndef NDEBUG
  InitValidation();
#endif

  createInfo.enabledLayerCount       = static_cast<uint32_t>(m_EnabledLayers.size());
  createInfo.enabledExtensionCount   = static_cast<uint32_t>(m_EnabledExtensions.size());
  createInfo.ppEnabledLayerNames     = m_EnabledLayers.data();
  createInfo.ppEnabledExtensionNames = m_EnabledExtensions.data();
  
  API_CALL(&vkCreateInstance, &createInfo, nullptr, &m_vkInstance);

#ifndef NDEBUG
  SetupDebugMessenger();
#endif

  m_device.Init(m_vkInstance);
}

void GraphicEngine::Cleanup()
{
#ifndef NDEBUG
    API_CALL(DestroyDebugUtilsMessengerEXT, m_vkInstance, m_debugMessenger, nullptr);
#endif

    API_CALL(vkDestroyInstance, m_vkInstance, nullptr);
}

