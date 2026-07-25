const std = @import("std");
const builtin = @import("builtin");
const wm = @import("./window_manager.zig");

const c = @cImport({
    @cDefine("VK_NO_PROTOTYPES", "1");
    @cInclude("vulkan/vulkan.h");
});

const allocator = std.heap.page_allocator;
const MAX_FRAMES_IN_FLIGHT = 2;

fn RequiredProc(comptime Proc: type) type {
    return @typeInfo(Proc).optional.child;
}

const VulkanApi = struct {
    vkGetInstanceProcAddr: RequiredProc(c.PFN_vkGetInstanceProcAddr),
    vkGetDeviceProcAddr: RequiredProc(c.PFN_vkGetDeviceProcAddr),
    vkCreateInstance: RequiredProc(c.PFN_vkCreateInstance),
    vkDestroyInstance: RequiredProc(c.PFN_vkDestroyInstance),
    vkEnumeratePhysicalDevices: RequiredProc(c.PFN_vkEnumeratePhysicalDevices),
    vkGetPhysicalDeviceQueueFamilyProperties: RequiredProc(c.PFN_vkGetPhysicalDeviceQueueFamilyProperties),
    vkEnumerateDeviceExtensionProperties: RequiredProc(c.PFN_vkEnumerateDeviceExtensionProperties),
    vkGetPhysicalDeviceSurfaceSupportKHR: RequiredProc(c.PFN_vkGetPhysicalDeviceSurfaceSupportKHR),
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR: RequiredProc(c.PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR),
    vkGetPhysicalDeviceSurfaceFormatsKHR: RequiredProc(c.PFN_vkGetPhysicalDeviceSurfaceFormatsKHR),
    vkCreateDevice: RequiredProc(c.PFN_vkCreateDevice),
    vkDestroySurfaceKHR: RequiredProc(c.PFN_vkDestroySurfaceKHR),
    vkDestroyDevice: RequiredProc(c.PFN_vkDestroyDevice),
    vkGetDeviceQueue: RequiredProc(c.PFN_vkGetDeviceQueue),
    vkCreateSwapchainKHR: RequiredProc(c.PFN_vkCreateSwapchainKHR),
    vkDestroySwapchainKHR: RequiredProc(c.PFN_vkDestroySwapchainKHR),
    vkGetSwapchainImagesKHR: RequiredProc(c.PFN_vkGetSwapchainImagesKHR),
    vkCreateCommandPool: RequiredProc(c.PFN_vkCreateCommandPool),
    vkDestroyCommandPool: RequiredProc(c.PFN_vkDestroyCommandPool),
    vkAllocateCommandBuffers: RequiredProc(c.PFN_vkAllocateCommandBuffers),
    vkCreateSemaphore: RequiredProc(c.PFN_vkCreateSemaphore),
    vkDestroySemaphore: RequiredProc(c.PFN_vkDestroySemaphore),
    vkCreateFence: RequiredProc(c.PFN_vkCreateFence),
    vkDestroyFence: RequiredProc(c.PFN_vkDestroyFence),
    vkDeviceWaitIdle: RequiredProc(c.PFN_vkDeviceWaitIdle),
    vkAcquireNextImageKHR: RequiredProc(c.PFN_vkAcquireNextImageKHR),
    vkWaitForFences: RequiredProc(c.PFN_vkWaitForFences),
    vkResetFences: RequiredProc(c.PFN_vkResetFences),
    vkResetCommandBuffer: RequiredProc(c.PFN_vkResetCommandBuffer),
    vkBeginCommandBuffer: RequiredProc(c.PFN_vkBeginCommandBuffer),
    vkEndCommandBuffer: RequiredProc(c.PFN_vkEndCommandBuffer),
    vkCmdClearColorImage: RequiredProc(c.PFN_vkCmdClearColorImage),
    vkCmdPipelineBarrier: RequiredProc(c.PFN_vkCmdPipelineBarrier),
    vkQueueSubmit: RequiredProc(c.PFN_vkQueueSubmit),
    vkQueuePresentKHR: RequiredProc(c.PFN_vkQueuePresentKHR),

    fn loadGlobal(self: *VulkanApi, get_instance_proc_addr: RequiredProc(c.PFN_vkGetInstanceProcAddr)) !void {
        self.vkGetInstanceProcAddr = get_instance_proc_addr;
        self.vkCreateInstance = try loadInstanceProc(
            c.PFN_vkCreateInstance,
            get_instance_proc_addr,
            null,
            "vkCreateInstance",
        );
    }

    fn loadInstance(self: *VulkanApi, instance: c.VkInstance) !void {
        const get = self.vkGetInstanceProcAddr;
        self.vkGetDeviceProcAddr = try loadInstanceProc(c.PFN_vkGetDeviceProcAddr, get, instance, "vkGetDeviceProcAddr");
        self.vkEnumeratePhysicalDevices = try loadInstanceProc(c.PFN_vkEnumeratePhysicalDevices, get, instance, "vkEnumeratePhysicalDevices");
        self.vkGetPhysicalDeviceQueueFamilyProperties = try loadInstanceProc(c.PFN_vkGetPhysicalDeviceQueueFamilyProperties, get, instance, "vkGetPhysicalDeviceQueueFamilyProperties");
        self.vkEnumerateDeviceExtensionProperties = try loadInstanceProc(c.PFN_vkEnumerateDeviceExtensionProperties, get, instance, "vkEnumerateDeviceExtensionProperties");
        self.vkGetPhysicalDeviceSurfaceSupportKHR = try loadInstanceProc(c.PFN_vkGetPhysicalDeviceSurfaceSupportKHR, get, instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
        self.vkGetPhysicalDeviceSurfaceCapabilitiesKHR = try loadInstanceProc(c.PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR, get, instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
        self.vkGetPhysicalDeviceSurfaceFormatsKHR = try loadInstanceProc(c.PFN_vkGetPhysicalDeviceSurfaceFormatsKHR, get, instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
        self.vkCreateDevice = try loadInstanceProc(c.PFN_vkCreateDevice, get, instance, "vkCreateDevice");
        self.vkDestroySurfaceKHR = try loadInstanceProc(c.PFN_vkDestroySurfaceKHR, get, instance, "vkDestroySurfaceKHR");
    }

    fn loadDevice(self: *VulkanApi, device: c.VkDevice) !void {
        const get = self.vkGetDeviceProcAddr;
        self.vkGetDeviceQueue = try loadDeviceProc(c.PFN_vkGetDeviceQueue, get, device, "vkGetDeviceQueue");
        self.vkCreateSwapchainKHR = try loadDeviceProc(c.PFN_vkCreateSwapchainKHR, get, device, "vkCreateSwapchainKHR");
        self.vkDestroySwapchainKHR = try loadDeviceProc(c.PFN_vkDestroySwapchainKHR, get, device, "vkDestroySwapchainKHR");
        self.vkGetSwapchainImagesKHR = try loadDeviceProc(c.PFN_vkGetSwapchainImagesKHR, get, device, "vkGetSwapchainImagesKHR");
        self.vkCreateCommandPool = try loadDeviceProc(c.PFN_vkCreateCommandPool, get, device, "vkCreateCommandPool");
        self.vkDestroyCommandPool = try loadDeviceProc(c.PFN_vkDestroyCommandPool, get, device, "vkDestroyCommandPool");
        self.vkAllocateCommandBuffers = try loadDeviceProc(c.PFN_vkAllocateCommandBuffers, get, device, "vkAllocateCommandBuffers");
        self.vkCreateSemaphore = try loadDeviceProc(c.PFN_vkCreateSemaphore, get, device, "vkCreateSemaphore");
        self.vkDestroySemaphore = try loadDeviceProc(c.PFN_vkDestroySemaphore, get, device, "vkDestroySemaphore");
        self.vkCreateFence = try loadDeviceProc(c.PFN_vkCreateFence, get, device, "vkCreateFence");
        self.vkDestroyFence = try loadDeviceProc(c.PFN_vkDestroyFence, get, device, "vkDestroyFence");
        self.vkDeviceWaitIdle = try loadDeviceProc(c.PFN_vkDeviceWaitIdle, get, device, "vkDeviceWaitIdle");
        self.vkAcquireNextImageKHR = try loadDeviceProc(c.PFN_vkAcquireNextImageKHR, get, device, "vkAcquireNextImageKHR");
        self.vkWaitForFences = try loadDeviceProc(c.PFN_vkWaitForFences, get, device, "vkWaitForFences");
        self.vkResetFences = try loadDeviceProc(c.PFN_vkResetFences, get, device, "vkResetFences");
        self.vkResetCommandBuffer = try loadDeviceProc(c.PFN_vkResetCommandBuffer, get, device, "vkResetCommandBuffer");
        self.vkBeginCommandBuffer = try loadDeviceProc(c.PFN_vkBeginCommandBuffer, get, device, "vkBeginCommandBuffer");
        self.vkEndCommandBuffer = try loadDeviceProc(c.PFN_vkEndCommandBuffer, get, device, "vkEndCommandBuffer");
        self.vkCmdClearColorImage = try loadDeviceProc(c.PFN_vkCmdClearColorImage, get, device, "vkCmdClearColorImage");
        self.vkCmdPipelineBarrier = try loadDeviceProc(c.PFN_vkCmdPipelineBarrier, get, device, "vkCmdPipelineBarrier");
        self.vkQueueSubmit = try loadDeviceProc(c.PFN_vkQueueSubmit, get, device, "vkQueueSubmit");
        self.vkQueuePresentKHR = try loadDeviceProc(c.PFN_vkQueuePresentKHR, get, device, "vkQueuePresentKHR");
    }
};

var vulkan_api: VulkanApi = undefined;

fn loadInstanceProc(
    comptime Proc: type,
    get: RequiredProc(c.PFN_vkGetInstanceProcAddr),
    instance: c.VkInstance,
    name: [:0]const u8,
) !RequiredProc(Proc) {
    return @ptrCast(get(instance, name) orelse return error.MissingVulkanEntryPoint);
}

fn loadDeviceProc(
    comptime Proc: type,
    get: RequiredProc(c.PFN_vkGetDeviceProcAddr),
    device: c.VkDevice,
    name: [:0]const u8,
) !RequiredProc(Proc) {
    return @ptrCast(get(device, name) orelse return error.MissingVulkanEntryPoint);
}

const Vulkan = @This();

/// The top-level Vulkan instance used to load global Vulkan functionality.
instance: c.VkInstance,
/// The Vulkan presentation surface created by the selected window backend.
surface: c.VkSurfaceKHR,
/// The selected GPU that supports graphics, presentation, and swapchains.
physical_device: c.VkPhysicalDevice,
/// The logical device created from the selected physical device.
device: c.VkDevice,
/// The queue family index used for both graphics commands and presentation.
queue_family_index: u32,
/// The queue that submits graphics and transfer command buffers.
graphics_queue: c.VkQueue,
/// The queue that presents rendered swapchain images to the native surface.
present_queue: c.VkQueue,
/// The swapchain that owns the presentable images for the native surface.
swapchain: c.VkSwapchainKHR,
/// The image format selected for swapchain images.
swapchain_format: c.VkFormat,
/// The pixel size selected for swapchain images.
swapchain_extent: c.VkExtent2D,
/// The images retrieved from the swapchain and cleared each frame.
swapchain_images: []c.VkImage,
/// The command pool used to allocate per-swapchain-image command buffers.
command_pool: c.VkCommandPool,
/// Command buffers that record the clear operation for each swapchain image.
command_buffers: []c.VkCommandBuffer,
/// Semaphores signaled when swapchain images are ready to be rendered to.
image_available: [MAX_FRAMES_IN_FLIGHT]c.VkSemaphore,
/// Semaphores signaled when rendering is complete and images may be presented.
render_finished: [MAX_FRAMES_IN_FLIGHT]c.VkSemaphore,
/// Fences that keep CPU frame submission from reusing GPU work still in flight.
in_flight: [MAX_FRAMES_IN_FLIGHT]c.VkFence,
/// The rotating frame slot used to index synchronization objects.
current_frame: usize,

/// Allocates and initializes all Vulkan resources needed to render into a native window.
pub fn init(window_manager: *wm.WindowManager) !*Vulkan {
    const initial_extent = window_manager.extent();
    const get_instance_proc_addr_handle = if (builtin.os.tag == .windows)
        try window_manager.vulkanGetInstanceProcAddr()
    else
        @intFromPtr(@extern(RequiredProc(c.PFN_vkGetInstanceProcAddr), .{
            .name = "vkGetInstanceProcAddr",
        }));
    const get_instance_proc_addr = vulkanHandleFromU64(
        RequiredProc(c.PFN_vkGetInstanceProcAddr),
        get_instance_proc_addr_handle,
    );
    try vulkan_api.loadGlobal(get_instance_proc_addr);

    const self = try allocator.create(Vulkan);
    errdefer allocator.destroy(self);
    self.* = undefined;
    self.current_frame = 0;
    self.swapchain_images = &.{};
    self.command_buffers = &.{};
    self.image_available = [_]c.VkSemaphore{null} ** MAX_FRAMES_IN_FLIGHT;
    self.render_finished = [_]c.VkSemaphore{null} ** MAX_FRAMES_IN_FLIGHT;
    self.in_flight = [_]c.VkFence{null} ** MAX_FRAMES_IN_FLIGHT;

    self.instance = try createInstance(window_manager.vulkanSurfaceExtension());
    vulkan_api.vkDestroyInstance = try loadInstanceProc(
        c.PFN_vkDestroyInstance,
        vulkan_api.vkGetInstanceProcAddr,
        self.instance,
        "vkDestroyInstance",
    );
    errdefer vulkan_api.vkDestroyInstance(self.instance, null);
    try vulkan_api.loadInstance(self.instance);

    const surface_handle = try window_manager.createVulkanSurface(
        @intFromPtr(self.instance),
        get_instance_proc_addr_handle,
    );
    self.surface = vulkanHandleFromU64(c.VkSurfaceKHR, surface_handle);
    errdefer vulkan_api.vkDestroySurfaceKHR(self.instance, self.surface, null);

    const selected = try selectPhysicalDevice(self.instance, self.surface);
    self.physical_device = selected.device;
    self.queue_family_index = selected.queue_family_index;

    try self.createDevice();
    errdefer vulkan_api.vkDestroyDevice(self.device, null);

    try self.createSwapchain(initial_extent.width, initial_extent.height);
    errdefer vulkan_api.vkDestroySwapchainKHR(self.device, self.swapchain, null);

    try self.createCommands();
    errdefer vulkan_api.vkDestroyCommandPool(self.device, self.command_pool, null);

    try self.createSyncObjects();

    return self;
}

/// Waits for the device to idle and destroys all Vulkan resources owned by this object.
pub fn deinit(self: *Vulkan) void {
    _ = vulkan_api.vkDeviceWaitIdle(self.device);
    for (0..MAX_FRAMES_IN_FLIGHT) |i| {
        if (self.in_flight[i] != null) vulkan_api.vkDestroyFence(self.device, self.in_flight[i], null);
        if (self.render_finished[i] != null)
            vulkan_api.vkDestroySemaphore(
                self.device,
                self.render_finished[i],
                null,
            );
        if (self.image_available[i] != null)
            vulkan_api.vkDestroySemaphore(
                self.device,
                self.image_available[i],
                null,
            );
    }
    self.destroySwapchainResources();
    vulkan_api.vkDestroyDevice(self.device, null);
    vulkan_api.vkDestroySurfaceKHR(self.instance, self.surface, null);
    vulkan_api.vkDestroyInstance(self.instance, null);
    allocator.destroy(self);
}

/// Recreates swapchain-dependent resources after the native window changes size.
pub fn recreateSwapchain(self: *Vulkan, width: u32, height: u32) !void {
    _ = vulkan_api.vkDeviceWaitIdle(self.device);
    self.destroySwapchainResources();
    try self.createSwapchain(width, height);
    try self.createCommands();
}

/// Destroys the swapchain, image list, command pool, and command buffer list.
fn destroySwapchainResources(self: *Vulkan) void {
    if (self.command_buffers.len != 0) {
        allocator.free(self.command_buffers);
        self.command_buffers = &.{};
    }
    if (self.command_pool != null) {
        vulkan_api.vkDestroyCommandPool(self.device, self.command_pool, null);
        self.command_pool = null;
    }
    if (self.swapchain_images.len != 0) {
        allocator.free(self.swapchain_images);
        self.swapchain_images = &.{};
    }
    if (self.swapchain != null) {
        vulkan_api.vkDestroySwapchainKHR(self.device, self.swapchain, null);
        self.swapchain = null;
    }
}

/// Creates a Vulkan instance with the selected platform's surface extensions enabled.
fn createInstance(platform_surface_extension: [*:0]const u8) !c.VkInstance {
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
        platform_surface_extension,
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
    try check(vulkan_api.vkCreateInstance(&create_info, null, &instance));
    return instance;
}

const SelectedDevice = struct {
    device: c.VkPhysicalDevice,
    queue_family_index: u32,
};

/// Finds a physical device with VK_KHR_swapchain, graphics queue, and present support.
fn selectPhysicalDevice(instance: c.VkInstance, surface: c.VkSurfaceKHR) !SelectedDevice {
    var count: u32 = 0;
    try check(vulkan_api.vkEnumeratePhysicalDevices(instance, &count, null));
    if (count == 0) return error.NoVulkanPhysicalDevice;

    const devices = try allocator.alloc(c.VkPhysicalDevice, count);
    defer allocator.free(devices);
    try check(vulkan_api.vkEnumeratePhysicalDevices(instance, &count, devices.ptr));

    for (devices) |device| {
        if (!try hasDeviceExtension(device, c.VK_KHR_SWAPCHAIN_EXTENSION_NAME)) continue;

        var queue_count: u32 = 0;
        vulkan_api.vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, null);
        const queues = try allocator.alloc(c.VkQueueFamilyProperties, queue_count);
        defer allocator.free(queues);
        vulkan_api.vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, queues.ptr);

        for (queues, 0..) |queue, i| {
            if ((queue.queueFlags & c.VK_QUEUE_GRAPHICS_BIT) == 0) continue;
            var present_supported: c.VkBool32 = c.VK_FALSE;
            try check(vulkan_api.vkGetPhysicalDeviceSurfaceSupportKHR(
                device,
                @intCast(i),
                surface,
                &present_supported,
            ));
            if (present_supported == c.VK_TRUE) {
                return .{ .device = device, .queue_family_index = @intCast(i) };
            }
        }
    }

    return error.NoSuitableVulkanPhysicalDevice;
}

/// Returns true when the physical device exposes the requested device extension.
fn hasDeviceExtension(device: c.VkPhysicalDevice, extension_name: [*:0]const u8) !bool {
    var count: u32 = 0;
    try check(vulkan_api.vkEnumerateDeviceExtensionProperties(device, null, &count, null));
    const extensions = try allocator.alloc(c.VkExtensionProperties, count);
    defer allocator.free(extensions);
    try check(vulkan_api.vkEnumerateDeviceExtensionProperties(device, null, &count, extensions.ptr));
    for (extensions) |extension| {
        if (std.mem.orderZ(u8, @ptrCast(&extension.extensionName), extension_name) == .eq) {
            return true;
        }
    }
    return false;
}

/// Creates the logical device and retrieves the graphics/present queue handle.
/// Returns whether a recreate is required.
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
    try check(vulkan_api.vkCreateDevice(self.physical_device, &create_info, null, &self.device));
    vulkan_api.vkDestroyDevice = try loadDeviceProc(
        c.PFN_vkDestroyDevice,
        vulkan_api.vkGetDeviceProcAddr,
        self.device,
        "vkDestroyDevice",
    );
    errdefer vulkan_api.vkDestroyDevice(self.device, null);
    try vulkan_api.loadDevice(self.device);
    vulkan_api.vkGetDeviceQueue(self.device, self.queue_family_index, 0, &self.graphics_queue);
    self.present_queue = self.graphics_queue;
}

/// Chooses surface settings, creates the swapchain, and stores its images.
fn createSwapchain(self: *Vulkan, preferred_width: u32, preferred_height: u32) !void {
    var capabilities: c.VkSurfaceCapabilitiesKHR = undefined;
    try check(vulkan_api.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        self.physical_device,
        self.surface,
        &capabilities,
    ));

    var format_count: u32 = 0;
    try check(vulkan_api.vkGetPhysicalDeviceSurfaceFormatsKHR(
        self.physical_device,
        self.surface,
        &format_count,
        null,
    ));
    const formats = try allocator.alloc(c.VkSurfaceFormatKHR, format_count);
    defer allocator.free(formats);
    try check(vulkan_api.vkGetPhysicalDeviceSurfaceFormatsKHR(
        self.physical_device,
        self.surface,
        &format_count,
        formats.ptr,
    ));

    var chosen_format = formats[0];
    for (formats) |format| {
        if (format.format == c.VK_FORMAT_B8G8R8A8_SRGB and
            format.colorSpace == c.VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            chosen_format = format;
            break;
        }
    }

    var extent = capabilities.currentExtent;
    if (extent.width == std.math.maxInt(u32)) {
        extent.width = std.math.clamp(
            preferred_width,
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width,
        );
        extent.height = std.math.clamp(
            preferred_height,
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height,
        );
    }

    var image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount != 0 and image_count > capabilities.maxImageCount) {
        image_count = capabilities.maxImageCount;
    }

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
    try check(vulkan_api.vkCreateSwapchainKHR(self.device, &create_info, null, &self.swapchain));
    self.swapchain_format = chosen_format.format;
    self.swapchain_extent = extent;

    var actual_count: u32 = 0;
    try check(vulkan_api.vkGetSwapchainImagesKHR(self.device, self.swapchain, &actual_count, null));
    self.swapchain_images = try allocator.alloc(c.VkImage, actual_count);
    try check(vulkan_api.vkGetSwapchainImagesKHR(
        self.device,
        self.swapchain,
        &actual_count,
        self.swapchain_images.ptr,
    ));
}

/// Creates a command pool and allocates command buffers for swapchain rendering.
fn createCommands(self: *Vulkan) !void {
    const pool_info = c.VkCommandPoolCreateInfo{
        .sType = c.VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = null,
        .flags = c.VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = self.queue_family_index,
    };
    try check(vulkan_api.vkCreateCommandPool(self.device, &pool_info, null, &self.command_pool));

    self.command_buffers = try allocator.alloc(c.VkCommandBuffer, self.swapchain_images.len);
    const alloc_info = c.VkCommandBufferAllocateInfo{
        .sType = c.VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = null,
        .commandPool = self.command_pool,
        .level = c.VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = @intCast(self.command_buffers.len),
    };
    try check(vulkan_api.vkAllocateCommandBuffers(self.device, &alloc_info, self.command_buffers.ptr));
}

/// Creates the semaphores and fences used to synchronize frames in flight.
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
        try check(vulkan_api.vkCreateSemaphore(self.device, &semaphore_info, null, &self.image_available[i]));
        try check(vulkan_api.vkCreateSemaphore(self.device, &semaphore_info, null, &self.render_finished[i]));
        try check(vulkan_api.vkCreateFence(self.device, &fence_info, null, &self.in_flight[i]));
    }
}

/// Acquires a swapchain image, records a clear command, submits it, and presents the image.
pub fn drawFrame(self: *Vulkan) !bool {
    const frame = self.current_frame;
    try check(vulkan_api.vkWaitForFences(
        self.device,
        1,
        &self.in_flight[frame],
        c.VK_TRUE,
        std.math.maxInt(u64),
    ));

    var image_index: u32 = 0;
    const acquire = vulkan_api.vkAcquireNextImageKHR(
        self.device,
        self.swapchain,
        std.math.maxInt(u64),
        self.image_available[frame],
        null,
        &image_index,
    );
    if (acquire == c.VK_ERROR_OUT_OF_DATE_KHR) return true;
    try check(acquire);

    try check(vulkan_api.vkResetFences(self.device, 1, &self.in_flight[frame]));
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
    try check(vulkan_api.vkQueueSubmit(self.graphics_queue, 1, &submit_info, self.in_flight[frame]));

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
    const present = vulkan_api.vkQueuePresentKHR(self.present_queue, &present_info);
    const needs_recreate = present == c.VK_ERROR_OUT_OF_DATE_KHR or present == c.VK_SUBOPTIMAL_KHR;
    if (!needs_recreate) try check(present);

    self.current_frame = (self.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    return needs_recreate;
}

/// Records commands that transition a swapchain image, clear it, and prepare it for presentation.
fn recordClearCommands(command_buffer: c.VkCommandBuffer, image: c.VkImage) !void {
    try check(vulkan_api.vkResetCommandBuffer(command_buffer, 0));
    const begin_info = c.VkCommandBufferBeginInfo{
        .sType = c.VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = null,
        .flags = 0,
        .pInheritanceInfo = null,
    };
    try check(vulkan_api.vkBeginCommandBuffer(command_buffer, &begin_info));

    imageBarrier(
        command_buffer,
        image,
        c.VK_IMAGE_LAYOUT_UNDEFINED,
        c.VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        0,
        c.VK_ACCESS_TRANSFER_WRITE_BIT,
        c.VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        c.VK_PIPELINE_STAGE_TRANSFER_BIT,
    );

    const clear = c.VkClearColorValue{ .float32 = .{ 0.05, 0.10, 0.20, 1.0 } };
    const range = c.VkImageSubresourceRange{
        .aspectMask = c.VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };
    vulkan_api.vkCmdClearColorImage(
        command_buffer,
        image,
        c.VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        &clear,
        1,
        &range,
    );

    imageBarrier(
        command_buffer,
        image,
        c.VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        c.VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        c.VK_ACCESS_TRANSFER_WRITE_BIT,
        0,
        c.VK_PIPELINE_STAGE_TRANSFER_BIT,
        c.VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    );

    try check(vulkan_api.vkEndCommandBuffer(command_buffer));
}

/// Emits an image memory barrier for layout transitions and access synchronization.
fn imageBarrier(
    command_buffer: c.VkCommandBuffer,
    image: c.VkImage,
    old_layout: c.VkImageLayout,
    new_layout: c.VkImageLayout,
    src_access: c.VkAccessFlags,
    dst_access: c.VkAccessFlags,
    src_stage: c.VkPipelineStageFlags,
    dst_stage: c.VkPipelineStageFlags,
) void {
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
    vulkan_api.vkCmdPipelineBarrier(command_buffer, src_stage, dst_stage, 0, 0, null, 0, null, 1, &barrier);
}

/// Converts non-success Vulkan result codes into a Zig error.
fn vulkanHandleFromU64(comptime Handle: type, value: u64) Handle {
    return switch (@typeInfo(Handle)) {
        .optional, .pointer => @ptrFromInt(@as(usize, @intCast(value))),
        .int => @intCast(value),
        else => @compileError("unsupported Vulkan handle representation"),
    };
}

fn check(result: c.VkResult) !void {
    if (result != c.VK_SUCCESS) return error.VulkanError;
}
