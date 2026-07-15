/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include "../../src/magic_context_private.h"
#include <vulkan/vulkan.h>
#include <cstdio>
#include <cstring>
#include <vector>

struct MagicVulkanState {
    BoardSurfaceVulkanInterface board{};
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkImageUsageFlags usage = 0;
    VkExtent2D extent{};
    VkPhysicalDeviceFeatures2 features{};
    std::vector<const char *> instance_extensions;
    std::vector<const char *> device_extensions;
    uint32_t queue_family = 0;
    std::vector<VkImage> images;
    std::vector<VkSemaphore> render_semaphores;
    uint32_t frame_slot = 0;
    uint32_t active_image = UINT32_MAX;
    uint64_t surface_generation = 0;
    char version[32]{};
};

static MagicVulkanState *magic_vk(MagicContext *context) { return context ? static_cast<MagicVulkanState *>(context->backend_data) : nullptr; }
static uint64_t magic_vk_handle(const void *value) { return (uint64_t)(uintptr_t)value; }
static int magic_vk_has_extension(const std::vector<const char *> &extensions, const char *name) { for (const char *extension : extensions) if (!std::strcmp(extension, name)) return 1; return 0; }
static int magic_vk_extension_available(const std::vector<VkExtensionProperties> &extensions, const char *name) { for (const VkExtensionProperties &extension : extensions) if (!std::strcmp(extension.extensionName, name)) return 1; return 0; }
static const char *const magic_vk_portability_subset_extension = "VK_KHR_portability_subset";

static void magic_vk_destroy_surface(MagicVulkanState *state) {
    if (!state || state->surface == VK_NULL_HANDLE) return;
    if (state->board.destroy_surface) state->board.destroy_surface(state->board.user_data, state->instance, (uint64_t)(uintptr_t)state->surface);
    else vkDestroySurfaceKHR(state->instance, state->surface, nullptr);
    state->surface = VK_NULL_HANDLE;
}

static void magic_vk_destroy_swapchain(MagicVulkanState *state) {
    if (!state || state->device == VK_NULL_HANDLE) return;
    if (state->swapchain != VK_NULL_HANDLE) vkDeviceWaitIdle(state->device);
    for (VkSemaphore semaphore : state->render_semaphores) vkDestroySemaphore(state->device, semaphore, nullptr);
    state->render_semaphores.clear(); state->images.clear();
    if (state->swapchain != VK_NULL_HANDLE) vkDestroySwapchainKHR(state->device, state->swapchain, nullptr);
    state->swapchain = VK_NULL_HANDLE; state->active_image = UINT32_MAX;
}

static MagicResult magic_vk_create_swapchain(MagicVulkanState *state) {
    VkSurfaceCapabilitiesKHR capabilities{};
    uint32_t count = 0;
    if (!state || state->surface == VK_NULL_HANDLE) return MAGIC_ERROR_SURFACE;
    magic_vk_destroy_swapchain(state);
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(state->physical_device, state->surface, &capabilities) != VK_SUCCESS ||
        vkGetPhysicalDeviceSurfaceFormatsKHR(state->physical_device, state->surface, &count, nullptr) != VK_SUCCESS || !count) return MAGIC_ERROR_SURFACE;
    std::vector<VkSurfaceFormatKHR> formats(count);
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(state->physical_device, state->surface, &count, formats.data()) != VK_SUCCESS) return MAGIC_ERROR_SURFACE;
    VkSurfaceFormatKHR selected = formats.front();
    for (const VkSurfaceFormatKHR &candidate : formats) {
        if (candidate.format == VK_FORMAT_B8G8R8A8_UNORM || candidate.format == VK_FORMAT_R8G8B8A8_UNORM || candidate.format == VK_FORMAT_B8G8R8A8_SRGB || candidate.format == VK_FORMAT_R8G8B8A8_SRGB) { selected = candidate; break; }
    }
    if (selected.format != VK_FORMAT_B8G8R8A8_UNORM && selected.format != VK_FORMAT_R8G8B8A8_UNORM && selected.format != VK_FORMAT_B8G8R8A8_SRGB && selected.format != VK_FORMAT_R8G8B8A8_SRGB) return MAGIC_ERROR_UNAVAILABLE;
    VkSwapchainCreateInfoKHR create{};
    create.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create.surface = state->surface;
    create.minImageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount && create.minImageCount > capabilities.maxImageCount) create.minImageCount = capabilities.maxImageCount;
    create.imageFormat = selected.format; create.imageColorSpace = selected.colorSpace;
    create.imageExtent = capabilities.currentExtent;
    if (!create.imageExtent.width || create.imageExtent.width == UINT32_MAX) return MAGIC_ERROR_SURFACE;
    create.imageArrayLayers = 1;
    create.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) create.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) create.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    create.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; create.preTransform = capabilities.currentTransform;
    create.compositeAlpha = (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) ? VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR : VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    create.presentMode = VK_PRESENT_MODE_FIFO_KHR; create.clipped = VK_TRUE;
    if (vkCreateSwapchainKHR(state->device, &create, nullptr, &state->swapchain) != VK_SUCCESS) return MAGIC_ERROR_SURFACE;
    state->format = selected.format; state->usage = create.imageUsage; state->extent = create.imageExtent;
    if (vkGetSwapchainImagesKHR(state->device, state->swapchain, &count, nullptr) != VK_SUCCESS || !count) return MAGIC_ERROR_SURFACE;
    state->images.resize(count);
    if (vkGetSwapchainImagesKHR(state->device, state->swapchain, &count, state->images.data()) != VK_SUCCESS) return MAGIC_ERROR_SURFACE;
    VkSemaphoreCreateInfo semaphore{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    state->render_semaphores.resize(count + 1);
    for (uint32_t index = 0; index < count; ++index) {
        if (vkCreateSemaphore(state->device, &semaphore, nullptr, &state->render_semaphores[index]) != VK_SUCCESS) return MAGIC_ERROR_SURFACE;
    }
    if (vkCreateSemaphore(state->device, &semaphore, nullptr, &state->render_semaphores[count]) != VK_SUCCESS) return MAGIC_ERROR_SURFACE;
    ++state->surface_generation;
    return MAGIC_OK;
}

static MagicResult magic_vk_create_surface(MagicVulkanState *state) {
    uint64_t surface = 0;
    if (!state || !state->board.create_surface || state->board.create_surface(state->board.user_data, state->instance, &surface) != BOARD_OK) return MAGIC_ERROR_SURFACE;
    state->surface = (VkSurfaceKHR)(uintptr_t)surface;
    return MAGIC_OK;
}

extern "C" MagicResult magic_vulkan_backend_create(MagicContext *context, BoardNativeSurface *surface) {
    BoardSurfaceVulkanInterface board{};
    MagicVulkanState *state = new (std::nothrow) MagicVulkanState{};
    uint32_t extension_count = 0, device_count = 0;
    const char *const *extensions;
    if (!state || board_surface_query_interface(surface, BOARD_SURFACE_INTERFACE_VULKAN, BOARD_ABI_VERSION, &board, sizeof(board)) != BOARD_OK) { delete state; return MAGIC_ERROR_UNAVAILABLE; }
    extensions = board.required_instance_extensions(board.user_data, &extension_count);
    if (!extensions || !extension_count) { delete state; return MAGIC_ERROR_UNAVAILABLE; }
    state->instance_extensions.assign(extensions, extensions + extension_count);
    {
        uint32_t available_count = 0;
        if (vkEnumerateInstanceExtensionProperties(nullptr, &available_count, nullptr) != VK_SUCCESS) { delete state; return MAGIC_ERROR_SURFACE; }
        std::vector<VkExtensionProperties> available(available_count);
        if (vkEnumerateInstanceExtensionProperties(nullptr, &available_count, available.data()) != VK_SUCCESS) { delete state; return MAGIC_ERROR_SURFACE; }
        if (magic_vk_extension_available(available, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) && !magic_vk_has_extension(state->instance_extensions, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) state->instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        if (magic_vk_extension_available(available, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) && !magic_vk_has_extension(state->instance_extensions, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) state->instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO}; app.apiVersion = VK_API_VERSION_1_1;
    VkInstanceCreateInfo instance_info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO}; instance_info.pApplicationInfo = &app; instance_info.enabledExtensionCount = (uint32_t)state->instance_extensions.size(); instance_info.ppEnabledExtensionNames = state->instance_extensions.data();
    if (magic_vk_has_extension(state->instance_extensions, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) instance_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    if (vkCreateInstance(&instance_info, nullptr, &state->instance) != VK_SUCCESS) { delete state; return MAGIC_ERROR_SURFACE; }
    state->board = board;
    if (magic_vk_create_surface(state) != MAGIC_OK || vkEnumeratePhysicalDevices(state->instance, &device_count, nullptr) != VK_SUCCESS || !device_count) goto failure;
    {
        std::vector<VkPhysicalDevice> devices(device_count);
        if (vkEnumeratePhysicalDevices(state->instance, &device_count, devices.data()) != VK_SUCCESS) goto failure;
        for (VkPhysicalDevice device : devices) {
            uint32_t family_count = 0; vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, nullptr);
            std::vector<VkQueueFamilyProperties> families(family_count); vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, families.data());
            for (uint32_t index = 0; index < family_count; ++index) { VkBool32 present = VK_FALSE; vkGetPhysicalDeviceSurfaceSupportKHR(device, index, state->surface, &present); if ((families[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present) { state->physical_device = device; state->queue_family = index; break; } }
            if (state->physical_device != VK_NULL_HANDLE) break;
        }
    }
    if (state->physical_device == VK_NULL_HANDLE) goto failure;
    { VkPhysicalDeviceProperties properties{}; vkGetPhysicalDeviceProperties(state->physical_device, &properties); std::snprintf(state->version, sizeof(state->version), "%u.%u.%u", VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion), VK_VERSION_PATCH(properties.apiVersion)); }
    {
        uint32_t extension_count = 0; float priority = 1.0f;
        if (vkEnumerateDeviceExtensionProperties(state->physical_device, nullptr, &extension_count, nullptr) != VK_SUCCESS) goto failure;
        std::vector<VkExtensionProperties> available_extensions(extension_count);
        if (vkEnumerateDeviceExtensionProperties(state->physical_device, nullptr, &extension_count, available_extensions.data()) != VK_SUCCESS || !magic_vk_extension_available(available_extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) goto failure;
        state->device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        if (magic_vk_extension_available(available_extensions, magic_vk_portability_subset_extension)) state->device_extensions.push_back(magic_vk_portability_subset_extension);
        PFN_vkGetPhysicalDeviceFeatures2 get_features = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2>(vkGetInstanceProcAddr(state->instance, "vkGetPhysicalDeviceFeatures2"));
        if (!get_features) goto failure;
        state->features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        get_features(state->physical_device, &state->features);
        VkDeviceQueueCreateInfo queue{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO}; queue.queueFamilyIndex = state->queue_family; queue.queueCount = 1; queue.pQueuePriorities = &priority;
        VkDeviceCreateInfo info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO}; info.queueCreateInfoCount = 1; info.pQueueCreateInfos = &queue; info.enabledExtensionCount = (uint32_t)state->device_extensions.size(); info.ppEnabledExtensionNames = state->device_extensions.data(); info.pNext = &state->features;
        if (vkCreateDevice(state->physical_device, &info, nullptr, &state->device) != VK_SUCCESS) goto failure;
    }
    vkGetDeviceQueue(state->device, state->queue_family, 0, &state->queue);
    if (magic_vk_create_swapchain(state) != MAGIC_OK) goto failure;
    context->backend_data = state; return MAGIC_OK;
failure:
    magic_vk_destroy_swapchain(state); magic_vk_destroy_surface(state); if (state->device != VK_NULL_HANDLE) vkDestroyDevice(state->device, nullptr); if (state->instance != VK_NULL_HANDLE) vkDestroyInstance(state->instance, nullptr); delete state; return MAGIC_ERROR_SURFACE;
}

extern "C" void magic_vulkan_backend_destroy(MagicContext *context) {
    MagicVulkanState *state = magic_vk(context); if (!state) return;
    magic_vk_destroy_swapchain(state); magic_vk_destroy_surface(state); if (state->device != VK_NULL_HANDLE) vkDestroyDevice(state->device, nullptr); if (state->instance != VK_NULL_HANDLE) vkDestroyInstance(state->instance, nullptr); delete state; context->backend_data = nullptr;
}

extern "C" const char *magic_vulkan_backend_version(const MagicContext *context) { const MagicVulkanState *state = magic_vk(const_cast<MagicContext *>(context)); return state && state->version[0] ? state->version : "unknown"; }

extern "C" MagicResult magic_vulkan_backend_resize(MagicContext *context, uint32_t, uint32_t, float) {
    MagicVulkanState *state = magic_vk(context); if (!state) return MAGIC_ERROR_INVALID_ARGUMENT;
    magic_vk_destroy_swapchain(state); magic_vk_destroy_surface(state);
    if (magic_vk_create_surface(state) != MAGIC_OK) return MAGIC_ERROR_SURFACE;
    return magic_vk_create_swapchain(state);
}

extern "C" MagicResult magic_vulkan_backend_begin_frame(MagicContext *context, MagicFrame *frame) {
    MagicVulkanState *state = magic_vk(context); uint32_t image = 0; VkSemaphore acquire = VK_NULL_HANDLE; VkSemaphoreCreateInfo semaphore{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    if (!state || !frame || state->active_image != UINT32_MAX || state->swapchain == VK_NULL_HANDLE) return MAGIC_ERROR_INVALID_ARGUMENT;
    state->frame_slot = (uint32_t)(frame->sequence % state->render_semaphores.size());
    if (vkCreateSemaphore(state->device, &semaphore, nullptr, &acquire) != VK_SUCCESS) return MAGIC_ERROR_SURFACE;
    VkResult result = vkAcquireNextImageKHR(state->device, state->swapchain, UINT64_MAX, acquire, VK_NULL_HANDLE, &image);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) { vkDestroySemaphore(state->device, acquire, nullptr); if (magic_vk_create_swapchain(state) != MAGIC_OK) return MAGIC_ERROR_SURFACE; return MAGIC_ERROR_SURFACE; }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { vkDestroySemaphore(state->device, acquire, nullptr); return MAGIC_ERROR_SURFACE; }
    state->active_image = image;
    MagicVulkanInterop interop{};
    interop.struct_size = sizeof(interop); interop.abi_version = MAGIC_ABI_VERSION; interop.instance = magic_vk_handle(state->instance); interop.physical_device = magic_vk_handle(state->physical_device); interop.device = magic_vk_handle(state->device); interop.queue = magic_vk_handle(state->queue); interop.image = (uint64_t)state->images[image]; interop.acquire_semaphore = (uint64_t)acquire; interop.render_complete_semaphore = (uint64_t)state->render_semaphores[state->frame_slot]; interop.surface_generation = state->surface_generation; interop.device_features = &state->features; interop.queue_family = state->queue_family; interop.image_format = (uint32_t)state->format; interop.image_usage = (uint32_t)state->usage; interop.width = state->extent.width; interop.height = state->extent.height; interop.scale = 1.0f; interop.instance_extensions = state->instance_extensions.data(); interop.instance_extension_count = (uint32_t)state->instance_extensions.size(); interop.device_extensions = state->device_extensions.data(); interop.device_extension_count = (uint32_t)state->device_extensions.size();
    frame->vulkan = interop;
    return MAGIC_OK;
}

extern "C" MagicResult magic_vulkan_backend_end_frame(MagicContext *context, MagicFrame *) {
    MagicVulkanState *state = magic_vk(context); if (!state || state->active_image == UINT32_MAX) return MAGIC_ERROR_INVALID_ARGUMENT;
    VkPresentInfoKHR present{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR}; VkSemaphore wait = state->render_semaphores[state->frame_slot]; VkSwapchainKHR swapchain = state->swapchain; uint32_t image = state->active_image;
    present.waitSemaphoreCount = 1; present.pWaitSemaphores = &wait; present.swapchainCount = 1; present.pSwapchains = &swapchain; present.pImageIndices = &image;
    VkResult result = vkQueuePresentKHR(state->queue, &present); state->active_image = UINT32_MAX;
    if (result == VK_ERROR_OUT_OF_DATE_KHR) { (void)magic_vk_create_swapchain(state); return MAGIC_OK; }
    return (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) ? MAGIC_OK : MAGIC_ERROR_SURFACE;
}
