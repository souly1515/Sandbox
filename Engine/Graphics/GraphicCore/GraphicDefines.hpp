#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>

template<typename T, typename... Args>
void API_CALL(T t, Args... args)
{
#ifdef NDEBUG
  t(args...);
#else
  if constexpr (std::is_same<decltype(t(args...)), VkResult>::value)
  {
    VkResult res = t(args...);
    if (res != VK_SUCCESS) {
      throw std::runtime_error("API error" + res);
    }
  }
  else
  {
    t(args...);
  }
#endif
}

const uint32_t DISPLAY_WIDTH = 800;
const uint32_t DISPLAY_HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

