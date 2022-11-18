#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <stdio.h>  // printf, fprintf
#include <stdlib.h> // abort

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to
// maximize ease of testing and compatibility with old VS compilers. To link
// with VS2010-era libraries, VS2015+ requires linking with
// legacy_stdio_definitions.lib, which we do using this pragma. Your own project
// should not be affected, as you are likely to link with a newer binary of GLFW
// that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) &&                                 \
    !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

class mainWindow {

public:
  void init();

  void run();

  void cleanUp();

private:
  GLFWwindow *window_;
  ImGui_ImplVulkanH_Window *wd_;

  VkAllocationCallbacks *gAllocator_ = NULL;
  VkInstance gInstance_ = VK_NULL_HANDLE;
  VkPhysicalDevice gPhysicalDevice_ = VK_NULL_HANDLE;
  VkDevice gDevice_ = VK_NULL_HANDLE;
  uint32_t gQueueFamily_ = (uint32_t)-1;
  VkQueue gQueue_ = VK_NULL_HANDLE;
  VkDebugReportCallbackEXT gDebugReport_ = VK_NULL_HANDLE;
  VkPipelineCache gPipelineCache_ = VK_NULL_HANDLE;
  VkDescriptorPool gDescriptorPool_ = VK_NULL_HANDLE;

  ImGui_ImplVulkanH_Window mainWindowData_;
  int minImageCount_ = 2;
  bool swapChainRebuild_ = false;

  static void checkVKResult(VkResult err);

  void setupVulkan(const char **extensions, uint32_t extensions_count);

  void setupVulkanWindow(ImGui_ImplVulkanH_Window *wd, VkSurfaceKHR surface,
                         int width, int height);

  void cleanupVulkan();

  void cleanupVulkanWindow();

  void frameRender(ImGui_ImplVulkanH_Window *wd, ImDrawData *draw_data);

  void framePresent(ImGui_ImplVulkanH_Window *wd);

  static void glfw_error_callback(int error, const char *description);
};
