#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>

#ifndef NDEBUG
#include <iostream>
#endif

template<typename T, typename... Args>
VkResult API_CALL(T t, Args&&... args)
{
    if constexpr (std::is_same<decltype(t(args...)), VkResult>::value)
    {
#ifdef NDEBUG
        return t(args...);
#else
        VkResult res = t(args...);
        if (res != VK_SUCCESS)
        {
            std::cout << "API error " << res << std::endl;
        }
        return res;
#endif
    }
    else
    {
        t(args...);
        return VK_SUCCESS;
    }
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



