//
// Created by Martin Sonntag on 01.03.22.
//

#include "mainWindow.hpp"

void mainWindow::init() {

  // Setup GLFW window
  glfwSetErrorCallback(glfw_error_callback);
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window_ =
      glfwCreateWindow(1280, 720, "Dear ImGui GLFW+Vulkan example", NULL, NULL);
  // Setup Vulkan
  glfwVulkanSupported();
  uint32_t extensions_count = 0;
  const char **extensions =
      glfwGetRequiredInstanceExtensions(&extensions_count);
  setupVulkan(extensions, extensions_count);

  // Create Window Surface
  VkSurfaceKHR surface;
  VkResult err =
      glfwCreateWindowSurface(gInstance_, window_, gAllocator_, &surface);
  checkVKResult(err);

  // Create Framebuffers
  int w, h;
  glfwGetFramebufferSize(window_, &w, &h);
  wd_ = &mainWindowData_;
  setupVulkanWindow(wd_, surface, w, h);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad
  // Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport /
                                                      // Platform Windows
  // io.ConfigViewportsNoAutoMerge = true;
  // io.ConfigViewportsNoTaskBarIcon = true;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsClassic();

  // When viewports are enabled we tweak WindowRounding/WindowBg so platform
  // windows can look identical to regular ones.
  ImGuiStyle &style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForVulkan(window_, true);
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = gInstance_;
  init_info.PhysicalDevice = gPhysicalDevice_;
  init_info.Device = gDevice_;
  init_info.Device = gDevice_;
  init_info.QueueFamily = gQueueFamily_;
  init_info.Queue = gQueue_;
  init_info.PipelineCache = gPipelineCache_;
  init_info.DescriptorPool = gDescriptorPool_;
  init_info.Subpass = 0;
  init_info.MinImageCount = minImageCount_;
  init_info.ImageCount = wd_->ImageCount;
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.Allocator = gAllocator_;
  init_info.CheckVkResultFn = checkVKResult;
  ImGui_ImplVulkan_Init(&init_info, wd_->RenderPass);

  // Upload Fonts
  {
    // Use any command queue
    VkCommandPool command_pool = wd_->Frames[wd_->FrameIndex].CommandPool;
    VkCommandBuffer command_buffer = wd_->Frames[wd_->FrameIndex].CommandBuffer;

    err = vkResetCommandPool(gDevice_, command_pool, 0);
    checkVKResult(err);
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    err = vkBeginCommandBuffer(command_buffer, &begin_info);
    checkVKResult(err);

    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

    VkSubmitInfo end_info = {};
    end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    end_info.commandBufferCount = 1;
    end_info.pCommandBuffers = &command_buffer;
    err = vkEndCommandBuffer(command_buffer);
    checkVKResult(err);
    err = vkQueueSubmit(gQueue_, 1, &end_info, VK_NULL_HANDLE);
    checkVKResult(err);

    err = vkDeviceWaitIdle(gDevice_);
    checkVKResult(err);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
  }
}

void mainWindow::run() {
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  ImGuiIO &io = ImGui::GetIO();
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();

    // Resize swap chain?
    if (swapChainRebuild_) {
      int width, height;
      glfwGetFramebufferSize(window_, &width, &height);
      if (width > 0 && height > 0) {
        ImGui_ImplVulkan_SetMinImageCount(minImageCount_);
        ImGui_ImplVulkanH_CreateOrResizeWindow(
            gInstance_, gPhysicalDevice_, gDevice_, &mainWindowData_,
            gQueueFamily_, gAllocator_, width, height, minImageCount_);
        mainWindowData_.FrameIndex = 0;
        swapChainRebuild_ = false;
      }
    }

    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Hello, world!");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    // Rendering
    ImGui::Render();
    ImDrawData *main_draw_data = ImGui::GetDrawData();
    const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f ||
                                    main_draw_data->DisplaySize.y <= 0.0f);
    wd_->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
    wd_->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
    wd_->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
    wd_->ClearValue.color.float32[3] = clear_color.w;
    if (!main_is_minimized)
      frameRender(wd_, main_draw_data);

    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
    }
    // Present Main Platform Window
    if (!main_is_minimized)
      framePresent(wd_);
  }
}

void mainWindow::cleanUp() {

  auto err = vkDeviceWaitIdle(gDevice_);
  checkVKResult(err);
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  cleanupVulkanWindow();
  cleanupVulkan();

  glfwDestroyWindow(window_);
  glfwTerminate();
}

void mainWindow::checkVKResult(VkResult err) {
  if (err == 0)
    return;
  fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
  if (err < 0)
    abort();
}

void mainWindow::setupVulkan(const char **extensions,
                             uint32_t extensions_count) {
  VkResult err;
  // Create Vulkan Instance
  {
    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.enabledExtensionCount = extensions_count;
    create_info.ppEnabledExtensionNames = extensions;
    // Create Vulkan Instance without any debug feature
    err = vkCreateInstance(&create_info, gAllocator_, &gInstance_);
    checkVKResult(err);
    IM_UNUSED(gDebugReport_);
  }

  // Select GPU
  {
    uint32_t gpu_count;
    err = vkEnumeratePhysicalDevices(gInstance_, &gpu_count, NULL);
    checkVKResult(err);
    IM_ASSERT(gpu_count > 0);

    VkPhysicalDevice *gpus =
        (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * gpu_count);
    err = vkEnumeratePhysicalDevices(gInstance_, &gpu_count, gpus);
    checkVKResult(err);

    // If a number >1 of GPUs got reported, find discrete GPU if present, or use
    // first one available. This covers most common cases
    // (multi-gpu/integrated+dedicated graphics). Handling more complicated
    // setups (multiple dedicated GPUs) is out of scope of this sample.
    int use_gpu = 0;
    for (int i = 0; i < (int)gpu_count; i++) {
      VkPhysicalDeviceProperties properties;
      vkGetPhysicalDeviceProperties(gpus[i], &properties);
      if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        use_gpu = i;
        break;
      }
    }

    gPhysicalDevice_ = gpus[use_gpu];
    free(gpus);
  }

  // Select graphics queue family
  {
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(gPhysicalDevice_, &count, NULL);
    VkQueueFamilyProperties *queues = (VkQueueFamilyProperties *)malloc(
        sizeof(VkQueueFamilyProperties) * count);
    vkGetPhysicalDeviceQueueFamilyProperties(gPhysicalDevice_, &count, queues);
    for (uint32_t i = 0; i < count; i++)
      if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        gQueueFamily_ = i;
        break;
      }
    free(queues);
    IM_ASSERT(gQueueFamily_ != (uint32_t)-1);
  }

  // Create Logical Device (with 1 queue)
  {
    int device_extension_count = 1;
    const char *device_extensions[] = {"VK_KHR_swapchain"};
    const float queue_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_info[1] = {};
    queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[0].queueFamilyIndex = gQueueFamily_;
    queue_info[0].queueCount = 1;
    queue_info[0].pQueuePriorities = queue_priority;
    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount =
        sizeof(queue_info) / sizeof(queue_info[0]);
    create_info.pQueueCreateInfos = queue_info;
    create_info.enabledExtensionCount = device_extension_count;
    create_info.ppEnabledExtensionNames = device_extensions;
    err =
        vkCreateDevice(gPhysicalDevice_, &create_info, gAllocator_, &gDevice_);
    checkVKResult(err);
    vkGetDeviceQueue(gDevice_, gQueueFamily_, 0, &gQueue_);
  }

  // Create Descriptor Pool
  {
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    err = vkCreateDescriptorPool(gDevice_, &pool_info, gAllocator_,
                                 &gDescriptorPool_);
    checkVKResult(err);
  }
}

void mainWindow::setupVulkanWindow(ImGui_ImplVulkanH_Window *wd,
                                   VkSurfaceKHR surface, int width,
                                   int height) {
  wd->Surface = surface;

  // Check for WSI support
  VkBool32 res;
  vkGetPhysicalDeviceSurfaceSupportKHR(gPhysicalDevice_, gQueueFamily_,
                                       wd->Surface, &res);
  if (res != VK_TRUE) {
    fprintf(stderr, "Error no WSI support on physical device 0\n");
    exit(-1);
  }

  // Select Surface Format
  const VkFormat requestSurfaceImageFormat[] = {
      VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM,
      VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
  const VkColorSpaceKHR requestSurfaceColorSpace =
      VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
      gPhysicalDevice_, wd->Surface, requestSurfaceImageFormat,
      (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat),
      requestSurfaceColorSpace);

  // Select Present Mode
  VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
  wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(
      gPhysicalDevice_, wd->Surface, &present_modes[0],
      IM_ARRAYSIZE(present_modes));
  // printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

  // Create SwapChain, RenderPass, Framebuffer, etc.
  IM_ASSERT(minImageCount_ >= 2);
  ImGui_ImplVulkanH_CreateOrResizeWindow(gInstance_, gPhysicalDevice_, gDevice_,
                                         wd, gQueueFamily_, gAllocator_, width,
                                         height, minImageCount_);
}

void mainWindow::cleanupVulkan() {
  vkDestroyDescriptorPool(gDevice_, gDescriptorPool_, gAllocator_);
  vkDestroyDevice(gDevice_, gAllocator_);
  vkDestroyInstance(gInstance_, gAllocator_);
}

void mainWindow::cleanupVulkanWindow() {
  ImGui_ImplVulkanH_DestroyWindow(gInstance_, gDevice_, &mainWindowData_,
                                  gAllocator_);
}

void mainWindow::frameRender(ImGui_ImplVulkanH_Window *wd,
                             ImDrawData *draw_data) {
  VkResult err;

  VkSemaphore image_acquired_semaphore =
      wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
  VkSemaphore render_complete_semaphore =
      wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
  err = vkAcquireNextImageKHR(gDevice_, wd->Swapchain, UINT64_MAX,
                              image_acquired_semaphore, VK_NULL_HANDLE,
                              &wd->FrameIndex);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
    swapChainRebuild_ = true;
    return;
  }
  checkVKResult(err);

  ImGui_ImplVulkanH_Frame *fd = &wd->Frames[wd->FrameIndex];
  {
    err = vkWaitForFences(
        gDevice_, 1, &fd->Fence, VK_TRUE,
        UINT64_MAX); // wait indefinitely instead of periodically checking
    checkVKResult(err);

    err = vkResetFences(gDevice_, 1, &fd->Fence);
    checkVKResult(err);
  }
  {
    err = vkResetCommandPool(gDevice_, fd->CommandPool, 0);
    checkVKResult(err);
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
    checkVKResult(err);
  }
  {
    VkRenderPassBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = wd->RenderPass;
    info.framebuffer = fd->Framebuffer;
    info.renderArea.extent.width = wd->Width;
    info.renderArea.extent.height = wd->Height;
    info.clearValueCount = 1;
    info.pClearValues = &wd->ClearValue;
    vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
  }

  // Record dear imgui primitives into command buffer
  ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

  // Submit command buffer
  vkCmdEndRenderPass(fd->CommandBuffer);
  {
    VkPipelineStageFlags wait_stage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &image_acquired_semaphore;
    info.pWaitDstStageMask = &wait_stage;
    info.commandBufferCount = 1;
    info.pCommandBuffers = &fd->CommandBuffer;
    info.signalSemaphoreCount = 1;
    info.pSignalSemaphores = &render_complete_semaphore;

    err = vkEndCommandBuffer(fd->CommandBuffer);
    checkVKResult(err);
    err = vkQueueSubmit(gQueue_, 1, &info, fd->Fence);
    checkVKResult(err);
  }
}

void mainWindow::framePresent(ImGui_ImplVulkanH_Window *wd) {
  if (swapChainRebuild_)
    return;
  VkSemaphore render_complete_semaphore =
      wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
  VkPresentInfoKHR info = {};
  info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  info.waitSemaphoreCount = 1;
  info.pWaitSemaphores = &render_complete_semaphore;
  info.swapchainCount = 1;
  info.pSwapchains = &wd->Swapchain;
  info.pImageIndices = &wd->FrameIndex;
  VkResult err = vkQueuePresentKHR(gQueue_, &info);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
    swapChainRebuild_ = true;
    return;
  }
  checkVKResult(err);
  wd->SemaphoreIndex =
      (wd->SemaphoreIndex + 1) %
      wd->ImageCount; // Now we can use the next set of semaphores
}

void mainWindow::glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}
