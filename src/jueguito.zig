const builtin = @import("builtin");

const WindowManager = @import("./window_manager.zig");
const Vulkan = @import("./vulkan.zig");

const Jueguito = @This();

window_manager: WindowManager,
// TODO: generic renderer
renderer: *Vulkan,

pub fn init() !Jueguito {
    const backend: WindowManager.BackendType = if (builtin.os.tag == .windows)
        .windows
    else
        .wayland;
    var window_manager = try WindowManager.init(backend);
    errdefer window_manager.deinit();

    return .{
        .window_manager = window_manager,
        .renderer = try Vulkan.init(&window_manager),
    };
}

/// Game loop
pub fn wan(self: *Jueguito) !void {
    while (self.window_manager.isRunning()) {
        try self.window_manager.pollEvents();
        if (!self.window_manager.isRunning()) break;

        if (!self.window_manager.canRender()) continue;

        if (self.window_manager.takeResize()) |extent| {
            try self.renderer.recreateSwapchain(extent.width, extent.height);
        }

        if (try self.renderer.drawFrame()) {
            const extent = self.window_manager.extent();
            try self.renderer.recreateSwapchain(extent.width, extent.height);
        }
    }
}

pub fn deinit(self: *Jueguito) void {
    // The renderer owns the swapchain and Vulkan surface, so it must be torn
    // down before the platform backend destroys the native window.
    self.renderer.deinit();
    self.window_manager.deinit();
}
