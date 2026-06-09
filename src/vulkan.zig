const std = @import("std");
const wm = @import("./window_manager.zig");
const wayland = @import("wayland");
const wl = wayland.client.wl;

const c = @cImport({
    @cDefine("VK_USE_PLATFORM_WAYLAND_KHR", "1");
    @cInclude("vulkan/vulkan.h");
    @cInclude("wayland-client.h");
});

const allocator = std.heap.page_allocator;
const WIDTH: u32 = 128;
const HEIGHT: u32 = 128;
const MAX_FRAMES_IN_FLIGHT = 2;

const Vulkan = @This();

instance: c.VkInstance,
surface: c.VkSurfaceKHR,
physical_device: c.VkPhysicalDevice,
device: c.VkDevice,
queue_family_index: u32,
graphics_queue: c.VkQueue,
present_queue: c.VkQueue,
swapchain: c.VkSwapchainKHR,
swapchain_format: c.VkFormat,
swapchain_extent: c.VkExtent2D,
swapchain_images: []c.VkImage,
command_pool: c.VkCommandPool,
command_buffers: []c.VkCommandBuffer,
image_available: [MAX_FRAMES_IN_FLIGHT]c.VkSemaphore,
render_finished: [MAX_FRAMES_IN_FLIGHT]c.VkSemaphore,
in_flight: [MAX_FRAMES_IN_FLIGHT]c.VkFence,
current_frame: usize,

pub fn init(window: *wm.Window) !*Vulkan {
    const self = try allocator.create(Vulkan);
    errdefer allocator.destroy(self);
    self.* = undefined;
    self.current_frame = 0;
    self.swapchain_images = &.{};
    self.command_buffers = &.{};
    self.image_available = [_]c.VkSemaphore{null} ** MAX_FRAMES_IN_FLIGHT;
    self.render_finished = [_]c.VkSemaphore{null} ** MAX_FRAMES_IN_FLIGHT;
    self.in_flight = [_]c.VkFence{null} ** MAX_FRAMES_IN_FLIGHT;

    self.instance = try createInstance();
    errdefer _ = c.vkDestroyInstance(self.instance, null);

    self.surface = try createWaylandSurface(self.instance, window.display, window.surface);
    errdefer c.vkDestroySurfaceKHR(self.instance, self.surface, null);

    const selected = try selectPhysicalDevice(self.instance, self.surface);
    self.physical_device = selected.device;
    self.queue_family_index = selected.queue_family_index;

    try self.createDevice();
    errdefer c.vkDestroyDevice(self.device, null);

    try self.createSwapchain();
    errdefer c.vkDestroySwapchainKHR(self.device, self.swapchain, null);

    try self.createCommands();
    errdefer c.vkDestroyCommandPool(self.device, self.command_pool, null);

    try self.createSyncObjects();

    return self;
}

pub fn deinit(self: *Vulkan) void {
    _ = c.vkDeviceWaitIdle(self.device);
    for (0..MAX_FRAMES_IN_FLIGHT) |i| {
        if (self.in_flight[i] != null) c.vkDestroyFence(self.device, self.in_flight[i], null);
        if (self.render_finished[i] != null) c.vkDestroySemaphore(self.device, self.render_finished[i], null);
        if (self.image_available[i] != null) c.vkDestroySemaphore(self.device, self.image_available[i], null);
    }
    if (self.command_buffers.len != 0) allocator.free(self.command_buffers);
    c.vkDestroyCommandPool(self.device, self.command_pool, null);
    if (self.swapchain_images.len != 0) allocator.free(self.swapchain_images);
    c.vkDestroySwapchainKHR(self.device, self.swapchain, null);
    c.vkDestroyDevice(self.device, null);
    c.vkDestroySurfaceKHR(self.instance, self.surface, null);
    c.vkDestroyInstance(self.instance, null);
    allocator.destroy(self);
}

pub fn run(self: *Vulkan, window: *wm.Window) !void {
    while (window.running) {
        _ = window.display.dispatchPending();
        try self.drawFrame();
        _ = window.display.flush();
    }
    _ = c.vkDeviceWaitIdle(self.device);
}

fn createInstance() !c.VkInstance {
    const app_info = c.VkApplicationInfo{
        .sType = c.VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = null,
        .pApplicationName = "pokemon",
        .applicationVersion = c.VK_MAKE_VERSION(0, 0, 0),
        .pEngineName = "pokemon",
        .engineVersion = c.VK_MAKE_VERSION(0, 0, 0),
        .apiVersion = c.VK_API_VERSION_1_0,
    };
    const extensions = [_][*:0]const u8{
        c.VK_KHR_SURFACE_EXTENSION_NAME,
        c.VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
    };
    const create_info = c.VkInstanceCreateInfo{
        .sType = c.VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = null,
        .flags = 0,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = null,
        .enabledExtensionCount = extensions.len,
        .ppEnabledExtensionNames = &extensions,
    };
    var instance: c.VkInstance = null;
    try check(c.vkCreateInstance(&create_info, null, &instance));
    return instance;
}

fn createWaylandSurface(instance: c.VkInstance, display: *wl.Display, surface: *wl.Surface) !c.VkSurfaceKHR {
    const create_info = c.VkWaylandSurfaceCreateInfoKHR{
        .sType = c.VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .pNext = null,
        .flags = 0,
        .display = @ptrCast(display),
        .surface = @ptrCast(surface),
    };
    var vk_surface: c.VkSurfaceKHR = null;
    try check(c.vkCreateWaylandSurfaceKHR(instance, &create_info, null, &vk_surface));
    return vk_surface;
}

const SelectedDevice = struct {
    device: c.VkPhysicalDevice,
    queue_family_index: u32,
};

fn selectPhysicalDevice(instance: c.VkInstance, surface: c.VkSurfaceKHR) !SelectedDevice {
    var count: u32 = 0;
    try check(c.vkEnumeratePhysicalDevices(instance, &count, null));
    if (count == 0) return error.NoVulkanPhysicalDevice;

    const devices = try allocator.alloc(c.VkPhysicalDevice, count);
    defer allocator.free(devices);
    try check(c.vkEnumeratePhysicalDevices(instance, &count, devices.ptr));

    for (devices) |device| {
        if (!try hasDeviceExtension(device, c.VK_KHR_SWAPCHAIN_EXTENSION_NAME)) continue;

        var queue_count: u32 = 0;
        c.vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, null);
        const queues = try allocator.alloc(c.VkQueueFamilyProperties, queue_count);
        defer allocator.free(queues);
        c.vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, queues.ptr);

        for (queues, 0..) |queue, i| {
            if ((queue.queueFlags & c.VK_QUEUE_GRAPHICS_BIT) == 0) continue;
            var present_supported: c.VkBool32 = c.VK_FALSE;
            try check(c.vkGetPhysicalDeviceSurfaceSupportKHR(device, @intCast(i), surface, &present_supported));
            if (present_supported == c.VK_TRUE) {
                return .{ .device = device, .queue_family_index = @intCast(i) };
            }
        }
    }

    return error.NoSuitableVulkanPhysicalDevice;
}

fn hasDeviceExtension(device: c.VkPhysicalDevice, extension_name: [*:0]const u8) !bool {
    var count: u32 = 0;
    try check(c.vkEnumerateDeviceExtensionProperties(device, null, &count, null));
    const extensions = try allocator.alloc(c.VkExtensionProperties, count);
    defer allocator.free(extensions);
    try check(c.vkEnumerateDeviceExtensionProperties(device, null, &count, extensions.ptr));
    for (extensions) |extension| {
        if (std.mem.orderZ(u8, @ptrCast(&extension.extensionName), extension_name) == .eq) return true;
    }
    return false;
}

fn createDevice(self: *Vulkan) !void {
    const priority: f32 = 1.0;
    const queue_info = c.VkDeviceQueueCreateInfo{
        .sType = c.VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = null,
        .flags = 0,
        .queueFamilyIndex = self.queue_family_index,
        .queueCount = 1,
        .pQueuePriorities = &priority,
    };
    const extensions = [_][*:0]const u8{c.VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    const create_info = c.VkDeviceCreateInfo{
        .sType = c.VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = null,
        .flags = 0,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_info,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = null,
        .enabledExtensionCount = extensions.len,
        .ppEnabledExtensionNames = &extensions,
        .pEnabledFeatures = null,
    };
    try check(c.vkCreateDevice(self.physical_device, &create_info, null, &self.device));
    c.vkGetDeviceQueue(self.device, self.queue_family_index, 0, &self.graphics_queue);
    self.present_queue = self.graphics_queue;
}

fn createSwapchain(self: *Vulkan) !void {
    var capabilities: c.VkSurfaceCapabilitiesKHR = undefined;
    try check(c.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(self.physical_device, self.surface, &capabilities));

    var format_count: u32 = 0;
    try check(c.vkGetPhysicalDeviceSurfaceFormatsKHR(self.physical_device, self.surface, &format_count, null));
    const formats = try allocator.alloc(c.VkSurfaceFormatKHR, format_count);
    defer allocator.free(formats);
    try check(c.vkGetPhysicalDeviceSurfaceFormatsKHR(self.physical_device, self.surface, &format_count, formats.ptr));

    var chosen_format = formats[0];
    for (formats) |format| {
        if (format.format == c.VK_FORMAT_B8G8R8A8_SRGB and format.colorSpace == c.VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            chosen_format = format;
            break;
        }
    }

    var extent = capabilities.currentExtent;
    if (extent.width == std.math.maxInt(u32)) {
        extent.width = std.math.clamp(WIDTH, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std.math.clamp(HEIGHT, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }

    var image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount != 0 and image_count > capabilities.maxImageCount) image_count = capabilities.maxImageCount;

    const create_info = c.VkSwapchainCreateInfoKHR{
        .sType = c.VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = null,
        .flags = 0,
        .surface = self.surface,
        .minImageCount = image_count,
        .imageFormat = chosen_format.format,
        .imageColorSpace = chosen_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = c.VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = c.VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = null,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = c.VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = c.VK_PRESENT_MODE_FIFO_KHR,
        .clipped = c.VK_TRUE,
        .oldSwapchain = null,
    };
    try check(c.vkCreateSwapchainKHR(self.device, &create_info, null, &self.swapchain));
    self.swapchain_format = chosen_format.format;
    self.swapchain_extent = extent;

    var actual_count: u32 = 0;
    try check(c.vkGetSwapchainImagesKHR(self.device, self.swapchain, &actual_count, null));
    self.swapchain_images = try allocator.alloc(c.VkImage, actual_count);
    try check(c.vkGetSwapchainImagesKHR(self.device, self.swapchain, &actual_count, self.swapchain_images.ptr));
}

fn createCommands(self: *Vulkan) !void {
    const pool_info = c.VkCommandPoolCreateInfo{
        .sType = c.VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = null,
        .flags = c.VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = self.queue_family_index,
    };
    try check(c.vkCreateCommandPool(self.device, &pool_info, null, &self.command_pool));

    self.command_buffers = try allocator.alloc(c.VkCommandBuffer, self.swapchain_images.len);
    const alloc_info = c.VkCommandBufferAllocateInfo{
        .sType = c.VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = null,
        .commandPool = self.command_pool,
        .level = c.VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = @intCast(self.command_buffers.len),
    };
    try check(c.vkAllocateCommandBuffers(self.device, &alloc_info, self.command_buffers.ptr));
}

fn createSyncObjects(self: *Vulkan) !void {
    const semaphore_info = c.VkSemaphoreCreateInfo{
        .sType = c.VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = null,
        .flags = 0,
    };
    const fence_info = c.VkFenceCreateInfo{
        .sType = c.VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = null,
        .flags = c.VK_FENCE_CREATE_SIGNALED_BIT,
    };
    for (0..MAX_FRAMES_IN_FLIGHT) |i| {
        try check(c.vkCreateSemaphore(self.device, &semaphore_info, null, &self.image_available[i]));
        try check(c.vkCreateSemaphore(self.device, &semaphore_info, null, &self.render_finished[i]));
        try check(c.vkCreateFence(self.device, &fence_info, null, &self.in_flight[i]));
    }
}

fn drawFrame(self: *Vulkan) !void {
    const frame = self.current_frame;
    try check(c.vkWaitForFences(self.device, 1, &self.in_flight[frame], c.VK_TRUE, std.math.maxInt(u64)));

    var image_index: u32 = 0;
    const acquire = c.vkAcquireNextImageKHR(self.device, self.swapchain, std.math.maxInt(u64), self.image_available[frame], null, &image_index);
    if (acquire == c.VK_ERROR_OUT_OF_DATE_KHR) return;
    try check(acquire);

    try check(c.vkResetFences(self.device, 1, &self.in_flight[frame]));
    try recordClearCommands(self.command_buffers[image_index], self.swapchain_images[image_index]);

    const wait_stages = [_]c.VkPipelineStageFlags{c.VK_PIPELINE_STAGE_TRANSFER_BIT};
    const submit_info = c.VkSubmitInfo{
        .sType = c.VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = null,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &self.image_available[frame],
        .pWaitDstStageMask = &wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &self.command_buffers[image_index],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &self.render_finished[frame],
    };
    try check(c.vkQueueSubmit(self.graphics_queue, 1, &submit_info, self.in_flight[frame]));

    const present_info = c.VkPresentInfoKHR{
        .sType = c.VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = null,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &self.render_finished[frame],
        .swapchainCount = 1,
        .pSwapchains = &self.swapchain,
        .pImageIndices = &image_index,
        .pResults = null,
    };
    const present = c.vkQueuePresentKHR(self.present_queue, &present_info);
    if (present != c.VK_ERROR_OUT_OF_DATE_KHR and present != c.VK_SUBOPTIMAL_KHR) try check(present);

    self.current_frame = (self.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

fn recordClearCommands(command_buffer: c.VkCommandBuffer, image: c.VkImage) !void {
    try check(c.vkResetCommandBuffer(command_buffer, 0));
    const begin_info = c.VkCommandBufferBeginInfo{
        .sType = c.VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = null,
        .flags = 0,
        .pInheritanceInfo = null,
    };
    try check(c.vkBeginCommandBuffer(command_buffer, &begin_info));

    imageBarrier(command_buffer, image, c.VK_IMAGE_LAYOUT_UNDEFINED, c.VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, c.VK_ACCESS_TRANSFER_WRITE_BIT, c.VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, c.VK_PIPELINE_STAGE_TRANSFER_BIT);

    const clear = c.VkClearColorValue{ .float32 = .{ 0.05, 0.10, 0.20, 1.0 } };
    const range = c.VkImageSubresourceRange{
        .aspectMask = c.VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };
    c.vkCmdClearColorImage(command_buffer, image, c.VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear, 1, &range);

    imageBarrier(command_buffer, image, c.VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, c.VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, c.VK_ACCESS_TRANSFER_WRITE_BIT, 0, c.VK_PIPELINE_STAGE_TRANSFER_BIT, c.VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

    try check(c.vkEndCommandBuffer(command_buffer));
}

fn imageBarrier(command_buffer: c.VkCommandBuffer, image: c.VkImage, old_layout: c.VkImageLayout, new_layout: c.VkImageLayout, src_access: c.VkAccessFlags, dst_access: c.VkAccessFlags, src_stage: c.VkPipelineStageFlags, dst_stage: c.VkPipelineStageFlags) void {
    const barrier = c.VkImageMemoryBarrier{
        .sType = c.VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = null,
        .srcAccessMask = src_access,
        .dstAccessMask = dst_access,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = c.VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = c.VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = .{
            .aspectMask = c.VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    c.vkCmdPipelineBarrier(command_buffer, src_stage, dst_stage, 0, 0, null, 0, null, 1, &barrier);
}

fn check(result: c.VkResult) !void {
    if (result != c.VK_SUCCESS) return error.VulkanError;
}
