#include "vulkan/vulkan.h"
#include <assert.h>
#include <stdio.h>
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>

#define VK_CHECK(call)                                                         \
  do {                                                                         \
    VkResult result_ = call;                                                   \
    assert(result_ == VK_SUCCESS);                                             \
  } while (0)

VkInstance createInstance() {

  VkApplicationInfo appInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .apiVersion = VK_API_VERSION_1_1,
  };

  VkInstanceCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &appInfo,
  };

#ifdef _DEBUG
  const char *debugLayers[] = {"VK_LAYER_LUNARG_standard_validation"};

  createInfo.ppEnabledLayerNames = debugLayers;
  createInfo.enabledLayerCount = sizeof(debugLayers) / sizeof(debugLayers[0]);
#endif

  const char *extensions[] = {
      VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
      VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
      VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#endif
  };

  createInfo.ppEnabledExtensionNames = extensions;
  createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);

  VkInstance instance = 0;
  VK_CHECK(vkCreateInstance(&createInfo, 0, &instance));

  return instance;
}

VkPhysicalDevice pickPhysicalDevice(VkPhysicalDevice *physicalDevices,
                                    uint32_t physicalDeviceCount) {
  for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevices[i], &props);

    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      printf("Pick discrete GPU: %s\n", props.deviceName);
      return physicalDevices[i];
    }
  }

  if (physicalDeviceCount > 0) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevices[0], &props);

    printf("Pick fallback GPU: %s\n", props.deviceName);
    return physicalDevices[0];
  }

  printf("No physical devices available!");
  return 0;
}

VkDevice createDevice(VkInstance instance, VkPhysicalDevice physicalDevice,
                      uint32_t *familyIndex) {
  *familyIndex = 0;

  float queuePriorities[] = {1.0f};

  VkDeviceQueueCreateInfo queueInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = *familyIndex,
      .queueCount = 1,
      .pQueuePriorities = queuePriorities,
  };

  const char *extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  VkDeviceCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queueInfo,
      .ppEnabledExtensionNames = extensions,
      .enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]),
  };

  VkDevice device = 0;
  VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, 0, &device));

  return device;
}

VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow *window) {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  VkWin32SurfaceCreateInfoKHR createInfo = {
      .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
      .hinstance = GetModuleHandle(0),
      .hwnd = glfwGetWin32Window(window),
  };

  VkSurfaceKHR surface = 0;
  VK_CHECK(vkCreateWin32SurfaceKHR(instance, &createInfo, 0, &surface));
  return surface;
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
  VkWaylandSurfaceCreateInfoKHR createInfo = {
      .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
      .display = glfwGetWaylandDisplay(),
      .surface = glfwGetWaylandWindow(window),
  };

  VkSurfaceKHR surface = 0;
  VK_CHECK(vkCreateWaylandSurfaceKHR(instance, &createInfo, 0, &surface));
  return surface;

#else
  VkSurfaceKHR surface = 0;
  VK_CHECK(glfwCreateWindowSurface(instance, window, 0, &surface));
  return surface;
#endif
}

VkSwapchainKHR createSwapchain(VkDevice device, VkSurfaceKHR surface,
                               uint32_t familyIndex, uint32_t width,
                               uint32_t height) {
  VkSwapchainCreateInfoKHR createInfo = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = surface,
      .minImageCount = 2,
      .imageFormat = VK_FORMAT_R8G8B8A8_UNORM,
      .imageColorSpace = VK_COLOR_SPACE_HDR10_ST2084_EXT,
      .imageExtent = {.width = width, .height = height},
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .queueFamilyIndexCount = 1,
      .pQueueFamilyIndices = &familyIndex,
      .presentMode = VK_PRESENT_MODE_FIFO_KHR,
  };

  VkSwapchainKHR swapchain = 0;
  VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, 0, &swapchain));

  return swapchain;
}

VkSemaphore createSemaphore(VkDevice device) {
  VkSemaphoreCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };

  VkSemaphore semaphore = 0;
  VK_CHECK(vkCreateSemaphore(device, &createInfo, 0, &semaphore));

  return semaphore;
}

VkCommandPool createCommandPool(VkDevice device, uint32_t familyIndex) {
  VkCommandPoolCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
      .queueFamilyIndex = familyIndex,
  };

  VkCommandPool commandPool = 0;
  VK_CHECK(vkCreateCommandPool(device, &createInfo, 0, &commandPool));

  return commandPool;
}

int main() {

  int ok = glfwInit();
  assert(ok);

  VkInstance instance = createInstance();
  assert(instance);

  VkPhysicalDevice physicalDevices[8];
  uint32_t physicalDeviceCount =
      sizeof(physicalDevices) / sizeof(physicalDevices[0]);
  VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount,
                                      physicalDevices));

  VkPhysicalDevice physicalDevice =
      pickPhysicalDevice(physicalDevices, physicalDeviceCount);
  assert(physicalDevice);

  uint32_t familyIndex = 0;
  VkDevice device = createDevice(instance, physicalDevice, &familyIndex);
  assert(device);

  GLFWwindow *window =
      glfwCreateWindow(1024, 768, "window without windows", 0, 0);
  assert(window);

  VkSurfaceKHR surface = createSurface(instance, window);
  assert(surface);

  int windowWidth = 0, windowHeight = 0;
  glfwGetWindowSize(window, &windowWidth, &windowHeight);

  VkSwapchainKHR swapchain =
      createSwapchain(device, surface, familyIndex, windowWidth, windowHeight);
  assert(swapchain);

  VkSemaphore acquireSemaphore = createSemaphore(device);
  assert(acquireSemaphore);

  VkSemaphore releaseSemaphore = createSemaphore(device);
  assert(releaseSemaphore);

  VkQueue queue = 0;
  vkGetDeviceQueue(device, familyIndex, 0, &queue);

  VkImage swapchainImages[3];
  uint32_t swapchainImageCount =
      sizeof(swapchainImages) / sizeof(swapchainImages[0]);
  VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount,
                                   swapchainImages));

  VkCommandPool commandPool = createCommandPool(device, familyIndex);
  assert(commandPool);

  VkCommandBufferAllocateInfo allocateInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };

  VkCommandBuffer commandBuffer = 0;
  VK_CHECK(vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer));

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    uint32_t imageIndex = 0;
    VK_CHECK(vkAcquireNextImageKHR(device, swapchain, ~0ull, acquireSemaphore,
                                   VK_NULL_HANDLE, &imageIndex));

    VK_CHECK(vkResetCommandPool(device, commandPool, 0));

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    VkClearColorValue color = {1, 0, 38, 1};

    VkImageSubresourceRange range = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .levelCount = 1,
        .layerCount = 1,
    };

    vkCmdClearColorImage(commandBuffer, swapchainImages[imageIndex],
                         VK_IMAGE_LAYOUT_GENERAL, &color, 1, &range);

    VK_CHECK(vkEndCommandBuffer(commandBuffer));

    VkPipelineStageFlags submitStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &acquireSemaphore,
        .pWaitDstStageMask = &submitStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &releaseSemaphore,
    };

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &releaseSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &imageIndex,
    };

    VK_CHECK(vkQueuePresentKHR(queue, &presentInfo));

    VK_CHECK(vkDeviceWaitIdle(device));
  }

  glfwDestroyWindow(window);

  vkDestroyInstance(instance, 0);
}
