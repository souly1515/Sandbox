#include "Surface.h"
#include "Engine/PlatformManager.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

void Surface::Init(VkInstance instance)
{
  m_instance = instance;
  VkWin32SurfaceCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.hwnd = glfwGetWin32Window(PlatformManager::GetInstance().GetWindow());
  createInfo.hinstance = GetModuleHandle(nullptr);

  API_CALL(vkCreateWin32SurfaceKHR, m_instance, &createInfo, nullptr, &m_surface);
}

void Surface::Cleanup()
{
  vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
}
