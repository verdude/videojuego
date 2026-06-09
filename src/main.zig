const std = @import("std");

const window_manager = @import("./window_manager.zig");
const Vulkan = @import("./vulkan.zig");

pub fn main() !void {
    std.log.debug("Creating window", .{});
    const window = try window_manager.create_window();
    defer window.destroy();
    std.log.debug("Created window", .{});

    const vulkan = try Vulkan.init(window);
    defer vulkan.deinit();

    try vulkan.run(window);
}
