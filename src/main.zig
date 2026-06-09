const std = @import("std");

const Window = @import("./window_manager.zig").Window;
const Vulkan = @import("./vulkan.zig");

pub fn main() !void {
    std.log.debug("Creating window", .{});
    const window = try Window.create();
    defer window.destroy();
    std.log.debug("Created window", .{});

    const vulkan = try Vulkan.init(window);
    defer vulkan.deinit();

    try vulkan.run(window);
}
