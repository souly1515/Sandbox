#pragma once
#include "Includes/Defines.h"
#include "vulkan/vulkan.h"

#include <vector>

class GraphicEngine
{
  DefaultSingleton(GraphicEngine);

  VkInstance instance;

#ifndef NDEBUG
  const std::vector<const char*> validationLayers = {
      "VK_LAYER_KHRONOS_validation"
  };

  std::vector<VkExtensionProperties> m_extensions;
  std::vector<VkLayerProperties> m_layers;
  
  void InitValidation(std::vector<const char*>& enabledLayers, std::vector<const char*>& enabledExtensions);
#endif
public:
#ifndef NDEBUG
  bool IsLayerSupported(/*layer type*/);
#endif
  bool IsExtensionSupported(/*layer type*/);

  void Init();
};