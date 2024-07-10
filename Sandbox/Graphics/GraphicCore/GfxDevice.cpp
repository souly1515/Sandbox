#include "GfxDevice.h"
#include "GraphicDefines.hpp"
#include "Engine/PlatformManager.h"

#include <vector>
#include <map>
#include <set>

void GfxDevice::Init(VkInstance vkInstance)
{
    uint32_t deviceCount = 0;

    m_surface.Init(vkInstance);

    API_CALL(vkEnumeratePhysicalDevices, vkInstance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    API_CALL(vkEnumeratePhysicalDevices, vkInstance, &deviceCount, devices.data());
    // Use an ordered map to automatically sort candidates by increasing score
    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto& device : devices)
    {
        int score = ScoreDevice(device);
        if (score)
            candidates.insert(std::make_pair(score, device));
    }

    if (candidates.rbegin()->first > 0)
    {
        m_physicalDevice = candidates.rbegin()->second;
    }
    else
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    m_queueFamilies = GetQueueFamily(m_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    std::set<uint32_t> uniqueQueueFamilies = {
      m_queueFamilies.graphicsFamily.value(),
      m_queueFamilies.presentFamily.value(),
      m_queueFamilies.computeFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};

        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;

        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

    auto deviceExtensions = PrepareExtensions(m_physicalDevice);
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    API_CALL(vkCreateDevice, m_physicalDevice, &createInfo, nullptr, &m_device);

    API_CALL(vkGetDeviceQueue, m_device, m_queueFamilies.graphicsFamily.value(), 0, &m_graphicsQueue);
    API_CALL(vkGetDeviceQueue, m_device, m_queueFamilies.presentFamily.value(), 0, &m_presentQueue);
    API_CALL(vkGetDeviceQueue, m_device, m_queueFamilies.computeFamily.value(), 0, &m_computeQueue);


    VkSurfaceFormatKHR swapchainFormat = { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    m_swapChain.Init(*this, swapchainFormat, m_surface, PlatformManager::GetInstance().GetSize());
}

void GfxDevice::RegisterExtensions(const std::vector<const char*>& requiredExtensions, const std::vector<const char*>& optionalExtensions)
{
    m_requiredExtensions = requiredExtensions;
    m_optionalExtensions = optionalExtensions;
}

void GfxDevice::CleanUp()
{
    m_swapChain.Cleanup();
    m_surface.Cleanup();
    API_CALL(vkDestroyDevice, m_device, nullptr);
}


QueueFamilyIndices GfxDevice::GetQueueFamily(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    int i = 0;

    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;

        if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
            indices.computeFamily = i;

        VkBool32 presentSupport = false;
        API_CALL(vkGetPhysicalDeviceSurfaceSupportKHR, device, i, m_surface, &presentSupport);

        if (presentSupport)
            indices.presentFamily = i;

        if (indices.isComplete())
            break;
        i++;
    }

    return indices;
}

QueueFamilyIndices GfxDevice::GetQueueFamily()
{
    return m_queueFamilies;
}

int32_t GfxDevice::ScoreDevice(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    int32_t score = IsDeviceUsable(device);
    if (score < 0)
        return -1000;

    SwapChainSupportDetails swapChainSupport = m_surface.QuerySwapChainSupport(device);
    if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty())
        return -1000;

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 1000;

    score += deviceProperties.limits.maxImageDimension2D;

    QueueFamilyIndices queueFamily = GetQueueFamily(device);

    if (!queueFamily.isComplete())
        score = 0;

    return score;
}

int32_t GfxDevice::IsDeviceUsable(VkPhysicalDevice device)
{
    int32_t score = 0;
    uint32_t extensionCount;
    API_CALL(vkEnumerateDeviceExtensionProperties, device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    API_CALL(vkEnumerateDeviceExtensionProperties, device, nullptr, &extensionCount, availableExtensions.data());

    for (auto& reqExt : m_requiredExtensions)
    {
        bool found = false;
        for (auto& name : availableExtensions)
        {
            if (strcmp(name.extensionName, reqExt) == 0)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            return -1000;
        }
    }

    for (auto& reqExt : m_optionalExtensions)
    {
        bool found = false;
        for (auto& name : availableExtensions)
        {
            if (strcmp(name.extensionName, reqExt) == 0)
            {
                found = true;
                break;
            }
        }
        if (found)
        {
            score += 10;
        }
    }



    return score;
}

std::vector<const char*> GfxDevice::PrepareExtensions(VkPhysicalDevice device)
{
    std::vector<const char*> extensions = m_requiredExtensions;

    uint32_t extensionCount;
    API_CALL(vkEnumerateDeviceExtensionProperties, device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    API_CALL(vkEnumerateDeviceExtensionProperties, device, nullptr, &extensionCount, availableExtensions.data());

    for (auto& reqExt : m_optionalExtensions)
    {
        for (auto& ext : availableExtensions)
        {
            if (strcmp(ext.extensionName, reqExt) == 0)
            {
                extensions.push_back(ext.extensionName);
                break;
            }
        }
    }
    return extensions;
}
