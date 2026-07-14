/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_internal.h"

#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#include <android/log.h>
#include <android/native_window.h>
#include <vulkan/vulkan.h>

#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/GrVkExtensions.h"

#include <new>
#include <vector>

typedef struct TcAndroidVulkanContext {
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;
    uint32_t queue_family = 0;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkImageUsageFlags usage = 0;
    VkExtent2D extent = {};
    VkPhysicalDeviceFeatures2 features = {};
    GrVkExtensions extensions;
    sk_sp<GrDirectContext> skia;
    std::vector<VkImage> images;
    std::vector<sk_sp<SkSurface>> surfaces;
    std::vector<VkSemaphore> render_semaphores;
    uint32_t backbuffer = 0;
    uint32_t active_backbuffer = UINT32_MAX;
    uint32_t active_image = UINT32_MAX;
} TcAndroidVulkanContext;

static void tc_android_vk_log(const char* message, VkResult result) {
    __android_log_print(ANDROID_LOG_ERROR, "tc_runtime", "%s: %d", message, result);
}

static TcAndroidVulkanContext* tc_android_vk(TcGraphicsContext* context) {
    return context && context->api == TC_GRAPHICS_VULKAN ? static_cast<TcAndroidVulkanContext*>(context->surface) : nullptr;
}

static PFN_vkVoidFunction tc_android_vk_get_proc(const char* name, VkInstance instance, VkDevice device) {
    return device != VK_NULL_HANDLE ? vkGetDeviceProcAddr(device, name) : vkGetInstanceProcAddr(instance, name);
}

static void tc_android_vk_destroy_swapchain(TcAndroidVulkanContext* vk) {
    if (!vk || vk->device == VK_NULL_HANDLE) return;
    if (vk->swapchain != VK_NULL_HANDLE || !vk->surfaces.empty()) vkDeviceWaitIdle(vk->device);
    vk->surfaces.clear();
    for (VkSemaphore semaphore : vk->render_semaphores) vkDestroySemaphore(vk->device, semaphore, nullptr);
    vk->render_semaphores.clear();
    vk->images.clear();
    if (vk->swapchain != VK_NULL_HANDLE) vkDestroySwapchainKHR(vk->device, vk->swapchain, nullptr);
    vk->swapchain = VK_NULL_HANDLE;
    vk->active_backbuffer = UINT32_MAX;
    vk->active_image = UINT32_MAX;
}

static int tc_android_vk_create_swapchain(TcGraphicsContext* context) {
    TcAndroidVulkanContext* vk = tc_android_vk(context);
    if (!vk) return TC_ERROR_INVALID_ARGUMENT;
    tc_android_vk_destroy_swapchain(vk);

    VkSurfaceCapabilitiesKHR caps = {};
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk->physical_device, vk->surface, &caps);
    if (result != VK_SUCCESS) { tc_android_vk_log("vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed", result); return TC_ERROR_PLATFORM; }
    uint32_t format_count = 0;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(vk->physical_device, vk->surface, &format_count, nullptr);
    if (result != VK_SUCCESS || format_count == 0) return TC_ERROR_PLATFORM;
    std::vector<VkSurfaceFormatKHR> formats(format_count);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(vk->physical_device, vk->surface, &format_count, formats.data());
    if (result != VK_SUCCESS) return TC_ERROR_PLATFORM;
    VkSurfaceFormatKHR selected = formats[0];
    for (const VkSurfaceFormatKHR& candidate : formats) {
        if (candidate.format == VK_FORMAT_B8G8R8A8_UNORM || candidate.format == VK_FORMAT_R8G8B8A8_UNORM || candidate.format == VK_FORMAT_B8G8R8A8_SRGB || candidate.format == VK_FORMAT_R8G8B8A8_SRGB) { selected = candidate; break; }
    }
    if (selected.format != VK_FORMAT_B8G8R8A8_UNORM && selected.format != VK_FORMAT_R8G8B8A8_UNORM && selected.format != VK_FORMAT_B8G8R8A8_SRGB && selected.format != VK_FORMAT_R8G8B8A8_SRGB) return TC_ERROR_UNAVAILABLE;
    VkExtent2D extent = caps.currentExtent;
    if (extent.width == UINT32_MAX) {
        extent.width = (uint32_t)ANativeWindow_getWidth((ANativeWindow*)context->window);
        extent.height = (uint32_t)ANativeWindow_getHeight((ANativeWindow*)context->window);
        if (extent.width < caps.minImageExtent.width) extent.width = caps.minImageExtent.width;
        if (extent.width > caps.maxImageExtent.width) extent.width = caps.maxImageExtent.width;
        if (extent.height < caps.minImageExtent.height) extent.height = caps.minImageExtent.height;
        if (extent.height > caps.maxImageExtent.height) extent.height = caps.maxImageExtent.height;
    }
    uint32_t image_count = caps.minImageCount + 1;
    if (caps.maxImageCount && image_count > caps.maxImageCount) image_count = caps.maxImageCount;
    VkCompositeAlphaFlagBitsKHR composite = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    const VkCompositeAlphaFlagBitsKHR composite_options[] = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    for (VkCompositeAlphaFlagBitsKHR option : composite_options) {
        if (caps.supportedCompositeAlpha & option) { composite = option; break; }
    }
    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = vk->surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = selected.format;
    create_info.imageColorSpace = selected.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) create_info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) create_info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.preTransform = caps.currentTransform;
    create_info.compositeAlpha = composite;
    create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    create_info.clipped = VK_TRUE;
    result = vkCreateSwapchainKHR(vk->device, &create_info, nullptr, &vk->swapchain);
    if (result != VK_SUCCESS) { tc_android_vk_log("vkCreateSwapchainKHR failed", result); return TC_ERROR_PLATFORM; }
    vk->format = selected.format;
    vk->usage = create_info.imageUsage;
    vk->extent = extent;
    result = vkGetSwapchainImagesKHR(vk->device, vk->swapchain, &image_count, nullptr);
    if (result != VK_SUCCESS || image_count == 0) return TC_ERROR_PLATFORM;
    vk->images.resize(image_count);
    result = vkGetSwapchainImagesKHR(vk->device, vk->swapchain, &image_count, vk->images.data());
    if (result != VK_SUCCESS) return TC_ERROR_PLATFORM;
    SkColorType color_type = (vk->format == VK_FORMAT_B8G8R8A8_UNORM || vk->format == VK_FORMAT_B8G8R8A8_SRGB) ? kBGRA_8888_SkColorType : kRGBA_8888_SkColorType;
    vk->surfaces.resize(image_count);
    for (uint32_t i = 0; i < image_count; ++i) {
        GrVkImageInfo image_info;
        image_info.fImage = vk->images[i];
        image_info.fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
        image_info.fFormat = vk->format;
        image_info.fImageUsageFlags = vk->usage;
        image_info.fSampleCount = 1;
        image_info.fLevelCount = 1;
        image_info.fCurrentQueueFamily = vk->queue_family;
        GrBackendRenderTarget target((int)extent.width, (int)extent.height, image_info);
        vk->surfaces[i] = SkSurface::MakeFromBackendRenderTarget(vk->skia.get(), target, kTopLeft_GrSurfaceOrigin, color_type, nullptr, nullptr);
        if (!vk->surfaces[i]) return TC_ERROR_RENDERER;
    }
    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vk->render_semaphores.resize(image_count + 1);
    for (VkSemaphore& semaphore : vk->render_semaphores) {
        result = vkCreateSemaphore(vk->device, &semaphore_info, nullptr, &semaphore);
        if (result != VK_SUCCESS) return TC_ERROR_PLATFORM;
    }
    context->width = (int)extent.width;
    context->height = (int)extent.height;
    context->scale = 1.0f;
    return TC_OK;
}

extern "C" int tc_android_vk_context_create(void* native_window, TcGraphicsContext** out_context) {
    if (!native_window || !out_context) return TC_ERROR_INVALID_ARGUMENT;
    TcGraphicsContext* context = (TcGraphicsContext*)calloc(1, sizeof(*context));
    TcAndroidVulkanContext* vk = new (std::nothrow) TcAndroidVulkanContext();
    if (!context || !vk) { free(context); delete vk; return TC_ERROR_OUT_OF_MEMORY; }
    bool created = [&]() {
        const char* instance_extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME};
        VkApplicationInfo app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.apiVersion = VK_API_VERSION_1_1;
        VkInstanceCreateInfo instance_info = {};
        instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_info.pApplicationInfo = &app_info;
        instance_info.enabledExtensionCount = 2;
        instance_info.ppEnabledExtensionNames = instance_extensions;
        if (vkCreateInstance(&instance_info, nullptr, &vk->instance) != VK_SUCCESS) return false;
        VkAndroidSurfaceCreateInfoKHR surface_info = {};
        surface_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
        surface_info.window = (ANativeWindow*)native_window;
        if (vkCreateAndroidSurfaceKHR(vk->instance, &surface_info, nullptr, &vk->surface) != VK_SUCCESS) return false;
        uint32_t physical_count = 0;
        if (vkEnumeratePhysicalDevices(vk->instance, &physical_count, nullptr) != VK_SUCCESS || physical_count == 0) return false;
        std::vector<VkPhysicalDevice> physical_devices(physical_count);
        if (vkEnumeratePhysicalDevices(vk->instance, &physical_count, physical_devices.data()) != VK_SUCCESS) return false;
        for (VkPhysicalDevice physical_device : physical_devices) {
            uint32_t family_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &family_count, nullptr);
            std::vector<VkQueueFamilyProperties> families(family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &family_count, families.data());
            for (uint32_t i = 0; i < family_count; ++i) {
                VkBool32 present = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, vk->surface, &present);
                if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present) { vk->physical_device = physical_device; vk->queue_family = i; break; }
            }
            if (vk->physical_device != VK_NULL_HANDLE) break;
        }
        if (vk->physical_device == VK_NULL_HANDLE) return false;
        vk->features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        PFN_vkGetPhysicalDeviceFeatures2 get_physical_device_features2 =
            reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2>(vkGetInstanceProcAddr(vk->instance, "vkGetPhysicalDeviceFeatures2"));
        if (!get_physical_device_features2) return false;
        get_physical_device_features2(vk->physical_device, &vk->features);
        float priority = 1.0f;
        VkDeviceQueueCreateInfo queue_info = {};
        queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.queueFamilyIndex = vk->queue_family;
        queue_info.queueCount = 1;
        queue_info.pQueuePriorities = &priority;
        const char* device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        VkDeviceCreateInfo device_info = {};
        device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_info.queueCreateInfoCount = 1;
        device_info.pQueueCreateInfos = &queue_info;
        device_info.enabledExtensionCount = 1;
        device_info.ppEnabledExtensionNames = device_extensions;
        device_info.pNext = &vk->features;
        if (vkCreateDevice(vk->physical_device, &device_info, nullptr, &vk->device) != VK_SUCCESS) return false;
        vkGetDeviceQueue(vk->device, vk->queue_family, 0, &vk->queue);
        vk->extensions.init(tc_android_vk_get_proc, vk->instance, vk->physical_device, 2, instance_extensions, 1, device_extensions);
        GrVkBackendContext backend_context = {};
        backend_context.fInstance = vk->instance;
        backend_context.fPhysicalDevice = vk->physical_device;
        backend_context.fDevice = vk->device;
        backend_context.fQueue = vk->queue;
        backend_context.fGraphicsQueueIndex = vk->queue_family;
        backend_context.fMaxAPIVersion = VK_API_VERSION_1_1;
        backend_context.fVkExtensions = &vk->extensions;
        backend_context.fDeviceFeatures2 = &vk->features;
        backend_context.fGetProc = tc_android_vk_get_proc;
        vk->skia = GrDirectContext::MakeVulkan(backend_context);
        if (!vk->skia) return false;
        context->api = TC_GRAPHICS_VULKAN;
        context->window = native_window;
        context->surface = vk;
        return tc_android_vk_create_swapchain(context) == TC_OK;
    }();
    if (created) {
        *out_context = context;
        __android_log_print(ANDROID_LOG_INFO, "tc_runtime", "Vulkan 1.1 Skia context created");
        return TC_OK;
    }
    if (vk->device != VK_NULL_HANDLE) { tc_android_vk_destroy_swapchain(vk); vk->skia.reset(); vkDestroyDevice(vk->device, nullptr); }
    if (vk->surface != VK_NULL_HANDLE) vkDestroySurfaceKHR(vk->instance, vk->surface, nullptr);
    if (vk->instance != VK_NULL_HANDLE) vkDestroyInstance(vk->instance, nullptr);
    delete vk;
    free(context);
    return TC_ERROR_PLATFORM;
}

extern "C" int tc_android_vk_context_resize(TcGraphicsContext* context, int width, int height, float scale) {
    (void)width; (void)height; (void)scale;
    return tc_android_vk_create_swapchain(context);
}

extern "C" void* tc_android_vk_context_get_skia_context(TcGraphicsContext* context) {
    TcAndroidVulkanContext* vk = tc_android_vk(context);
    return vk && vk->skia ? vk->skia.get() : nullptr;
}

extern "C" void* tc_android_vk_begin_frame(TcGraphicsContext* context) {
    TcAndroidVulkanContext* vk = tc_android_vk(context);
    if (!vk || vk->swapchain == VK_NULL_HANDLE || vk->surfaces.empty() || vk->active_image != UINT32_MAX) return nullptr;
    uint32_t backbuffer = vk->backbuffer++ % (uint32_t)vk->render_semaphores.size();
    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkSemaphore acquire = VK_NULL_HANDLE;
    if (vkCreateSemaphore(vk->device, &semaphore_info, nullptr, &acquire) != VK_SUCCESS) return nullptr;
    uint32_t image = 0;
    VkResult result = vkAcquireNextImageKHR(vk->device, vk->swapchain, UINT64_MAX, acquire, VK_NULL_HANDLE, &image);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) tc_android_vk_create_swapchain(context);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { vkDestroySemaphore(vk->device, acquire, nullptr); return nullptr; }
    GrBackendSemaphore wait;
    wait.initVulkan(acquire);
    if (!vk->surfaces[image]->wait(1, &wait)) { vkDestroySemaphore(vk->device, acquire, nullptr); return nullptr; }
    vk->active_backbuffer = backbuffer;
    vk->active_image = image;
    return vk->surfaces[image].get();
}

extern "C" int tc_android_vk_end_frame(TcGraphicsContext* context, void* surface) {
    TcAndroidVulkanContext* vk = tc_android_vk(context);
    if (!vk || vk->active_image == UINT32_MAX || surface != vk->surfaces[vk->active_image].get()) return TC_ERROR_INVALID_ARGUMENT;
    GrBackendSemaphore signal;
    signal.initVulkan(vk->render_semaphores[vk->active_backbuffer]);
    GrFlushInfo flush_info = {};
    flush_info.fNumSemaphores = 1;
    flush_info.fSignalSemaphores = &signal;
    SkSurface* sk_surface = static_cast<SkSurface*>(surface);
    sk_surface->flush(SkSurface::BackendSurfaceAccess::kPresent, flush_info);
    if (!vk->skia->submit()) { vk->active_backbuffer = UINT32_MAX; vk->active_image = UINT32_MAX; return TC_ERROR_RENDERER; }
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &vk->render_semaphores[vk->active_backbuffer];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &vk->swapchain;
    present_info.pImageIndices = &vk->active_image;
    VkResult result = vkQueuePresentKHR(vk->queue, &present_info);
    vk->active_backbuffer = UINT32_MAX;
    vk->active_image = UINT32_MAX;
    if (result == VK_ERROR_OUT_OF_DATE_KHR) tc_android_vk_create_swapchain(context);
    return (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) ? TC_OK : TC_ERROR_PLATFORM;
}

extern "C" void tc_android_vk_context_destroy(TcGraphicsContext* context) {
    if (!context) return;
    TcAndroidVulkanContext* vk = tc_android_vk(context);
    if (vk) {
        tc_android_vk_destroy_swapchain(vk);
        vk->skia.reset();
        if (vk->surface != VK_NULL_HANDLE) vkDestroySurfaceKHR(vk->instance, vk->surface, nullptr);
        if (vk->device != VK_NULL_HANDLE) vkDestroyDevice(vk->device, nullptr);
        if (vk->instance != VK_NULL_HANDLE) vkDestroyInstance(vk->instance, nullptr);
        delete vk;
    }
    free(context);
}
#endif
