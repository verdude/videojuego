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
    // move them aqui
    try self.renderer.run(self.window);
}

pub fn deinit(self: *Jueguito) void {
    self.renderer.deinit();
    self.window.deinit();
}
