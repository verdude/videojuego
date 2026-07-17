const std = @import("std");

const Window = @import("./window_manager.zig").Window;
const Vulkan = @import("./vulkan.zig");

const Jueguito = @This();

window: *Window,
// TODO: generic renderer
renderer: *Vulkan,

pub fn init() !Jueguito {
    const window = try Window.init();
    errdefer window.deinit();

    return .{
        .window = window,
        .renderer = try Vulkan.init(window),
    };
}

/// Game loop
pub fn wan(self: *Jueguito) !void {
    while (self.window.running) {
        try self.window.pollEvents();

        if (self.window.takeResize()) |extent| {
            try self.renderer.recreateSwapchain(extent.width, extent.height);
        }

        if (try self.renderer.drawFrame()) {
            try self.renderer.recreateSwapchain(
                @intCast(self.window.width),
                @intCast(self.window.height),
            );
        }
    }
}

pub fn deinit(self: *Jueguito) void {
    self.renderer.deinit();
    self.window.deinit();
}
