#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <stdbool.h>

#define GGLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan_wayland.h>

#define VK_CHECK(call) do { \
    VkResult result_ = call; \
    assert(result_ == VK_SUCCESS); \
} while(0)

#ifndef ARRAYSIZE
#define ARRAYSIZE(array) (sizeof(array)/sizeof((array)[0]))
#endif

int main() {
    // Initialize GLFW
    int rc = glfwInit();
    assert(rc);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API         );
	GLFWwindow* window = glfwCreateWindow(800, 600, "niagara", 0, 0);
	assert(window);
    
	int windowWidth = 0, windowHeight = 0;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);


    // Create Vulkan instance
    VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &appInfo;

#ifdef _DEBUG
    const char* debugLayers[] = {"VK_LAYER_KHRONOS_validation"};
    createInfo.ppEnabledLayerNames = debugLayers;
    createInfo.enabledLayerCount = ARRAYSIZE(debugLayers);
#endif

    const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
        VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#endif
#ifndef NDEBUG
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
    };

    createInfo.ppEnabledExtensionNames = extensions;
    createInfo.enabledExtensionCount = ARRAYSIZE(extensions);

    VkInstance instance;
    VK_CHECK(vkCreateInstance(&createInfo, 0, &instance));

    // Enumerate physical devices
    VkPhysicalDevice physicalDevices[8];
    uint32_t physicalDeviceCount = ARRAYSIZE(physicalDevices);
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices));

    // Select physical device
    VkPhysicalDevice selectedPhysicalDevice = 0;
    VkPhysicalDevice discrete = 0;
    VkPhysicalDevice fallback = 0;

    for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &props);
        printf("GPU%d: %s\n", i, props.deviceName);

        if (!discrete && props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            discrete = physicalDevices[i];
        }
        if (!fallback) {
            fallback = physicalDevices[i];
        }
    }

    selectedPhysicalDevice = discrete ? discrete : fallback;
    if (selectedPhysicalDevice) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(selectedPhysicalDevice, &props);
        printf("Selected GPU: %s\n", props.deviceName);
    } else {
        printf("No suitable GPU found\n");
        exit(1);
    }

    // Get queue family properties
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(selectedPhysicalDevice, &queueFamilyCount, 0);
    VkQueueFamilyProperties* queueFamilies = malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(selectedPhysicalDevice, &queueFamilyCount, queueFamilies);
  uint32_t queuefamilyIndex =0;
    // Device queue creation
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queueCreateInfo.queueFamilyIndex = queuefamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    // Physical device features
    VkPhysicalDeviceFeatures deviceFeatures = {0};

    // Device creation
    const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.enabledExtensionCount = ARRAYSIZE(deviceExtensions);
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    VkDevice device;
    VK_CHECK(vkCreateDevice(selectedPhysicalDevice, &deviceCreateInfo, 0, &device));

// need different for other os
    VkWaylandSurfaceCreateInfoKHR surfacecreateInfo = { VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR };
    surfacecreateInfo.display = glfwGetWaylandDisplay();
    surfacecreateInfo.surface = glfwGetWaylandWindow(window);

    VkSurfaceKHR surface = 0;
    VK_CHECK(vkCreateWaylandSurfaceKHR(instance, &surfacecreateInfo, 0, &surface));
VkSemaphoreCreateInfo semInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    


    VkBool32 presentSupported = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(selectedPhysicalDevice, queuefamilyIndex, surface, &presentSupported));
	assert(presentSupported);

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(selectedPhysicalDevice, surface, &surfaceCapabilities));
    uint32_t formatCount = 0;
vkGetPhysicalDeviceSurfaceFormatsKHR(selectedPhysicalDevice, surface, &formatCount, NULL);
VkSurfaceFormatKHR* formats = malloc(formatCount * sizeof(VkSurfaceFormatKHR));
vkGetPhysicalDeviceSurfaceFormatsKHR(selectedPhysicalDevice, surface, &formatCount, formats);


    VkSwapchainKHR swapchain;
    VkSwapchainCreateInfoKHR swapchaincreateinfo = {};
    swapchaincreateinfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

    swapchaincreateinfo.surface = surface;
 //    If presentMode is not VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR nor VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR, then minImageCount must be greater than or equal to the
 // value returned in the minImageCount member of the VkSurfaceCapabilitiesKHR structure returned by vkGetPhysicalDeviceSurfaceCapabilitiesKHR 
  
 swapchaincreateinfo.minImageCount = surfaceCapabilities.minImageCount;
    // need to check dynamically for support 
    // swapchaincreateinfo.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
    // swapchaincreateinfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchaincreateinfo.imageFormat = formats[0].format;
    swapchaincreateinfo.imageColorSpace = formats[0].colorSpace;
    free(formats);
    swapchaincreateinfo.imageExtent.width = windowWidth;
    swapchaincreateinfo.imageExtent.height = windowHeight;
    swapchaincreateinfo.imageArrayLayers = 1;
    swapchaincreateinfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // exclusive is better concurrent for cross process sharing
    swapchaincreateinfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    // no transform 
    swapchaincreateinfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchaincreateinfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    // dynamic determine
    swapchaincreateinfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchaincreateinfo.clipped = VK_TRUE;
    swapchaincreateinfo.queueFamilyIndexCount = 1;
    swapchaincreateinfo.pQueueFamilyIndices = &queuefamilyIndex;
    VK_CHECK(vkCreateSwapchainKHR(device, &swapchaincreateinfo, 0, &swapchain));
  uint32_t imageCount = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, NULL));
    VkImage* swapchainImages = malloc(imageCount * sizeof(VkImage));
    VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages));

    VkSemaphore Semaphore;
    VK_CHECK(vkCreateSemaphore(device, &semInfo, 0, &Semaphore));

    
VkQueue queue;
    vkGetDeviceQueue(device, queuefamilyIndex, 0, &queue);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
  
    
    // Create new semaphore
    VkSemaphoreCreateInfo semInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VK_CHECK(vkCreateSemaphore(device, &semInfo, NULL, &Semaphore));
    
    uint32_t imageIndex = 0;
    VK_CHECK(vkAcquireNextImageKHR(device, swapchain, ~0ull,Semaphore, VK_NULL_HANDLE, &imageIndex));
      VkPresentInfoKHR  presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;
    VK_CHECK(vkQueuePresentKHR(queue, &presentInfo));
    // Wait for the image to be presented
    VK_CHECK(vkDeviceWaitIdle(device));
    // Destroy the semaphore
    vkDestroySemaphore(device, Semaphore, NULL);


    }

    // Cleanup
    vkDestroySemaphore(device, Semaphore, 0);

    vkDestroySurfaceKHR(instance, surface, 0);
    vkDestroyDevice(device, 0);
    vkDestroyInstance(instance, 0);
    glfwDestroyWindow(window);
    glfwTerminate();
     return 0;
}
