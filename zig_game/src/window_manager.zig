const builtin = @import("builtin");
const Wayland = @import("./wayland.zig").Window;
const Windows = @import("./windows.zig").Window;

pub const WindowManager = @This();

const is_windows = builtin.os.tag == .windows;

pub const BackendType = enum {
    wayland,
    windows,
};

/// Only the native backend carries storage on a given target. Both choices stay
/// visible so selecting an unsupported backend produces a normal runtime error.
pub const Backend = union(BackendType) {
    wayland: if (is_windows) void else *Wayland,
    windows: if (is_windows) *Windows else void,
};

pub const Extent = struct {
    width: u32,
    height: u32,
};

backend: Backend,

pub fn init(backend_type: BackendType) !WindowManager {
    return .{
        .backend = switch (backend_type) {
            .wayland => if (is_windows)
                return error.UnsupportedWindowBackend
            else
                .{ .wayland = try Wayland.init() },
            .windows => if (is_windows)
                .{ .windows = try Windows.init() }
            else
                return error.UnsupportedWindowBackend,
        },
    };
}

pub fn deinit(self: *WindowManager) void {
    switch (self.backend) {
        .wayland => |window| if (is_windows) unreachable else window.deinit(),
        .windows => |window| if (is_windows) window.deinit() else unreachable,
    }
}

pub fn pollEvents(self: *WindowManager) !void {
    switch (self.backend) {
        .wayland => |window| if (is_windows) unreachable else try window.pollEvents(),
        .windows => |window| if (is_windows) try window.pollEvents() else unreachable,
    }
}

pub fn takeResize(self: *WindowManager) ?Extent {
    return switch (self.backend) {
        .wayland => |window| if (is_windows) unreachable else convertExtent(window.takeResize()),
        .windows => |window| if (is_windows) convertExtent(window.takeResize()) else unreachable,
    };
}

pub fn isRunning(self: *const WindowManager) bool {
    return switch (self.backend) {
        .wayland => |window| if (is_windows) unreachable else window.running,
        .windows => |window| if (is_windows) window.running else unreachable,
    };
}

pub fn canRender(self: *const WindowManager) bool {
    return switch (self.backend) {
        .wayland => |window| if (is_windows) unreachable else window.canRender(),
        .windows => |window| if (is_windows) window.canRender() else unreachable,
    };
}

pub fn extent(self: *const WindowManager) Extent {
    return switch (self.backend) {
        .wayland => |window| if (is_windows) unreachable else .{
            .width = @intCast(window.width),
            .height = @intCast(window.height),
        },
        .windows => |window| if (is_windows) .{
            .width = window.width,
            .height = window.height,
        } else unreachable,
    };
}

pub fn vulkanSurfaceExtension(self: *const WindowManager) [*:0]const u8 {
    return switch (self.backend) {
        .wayland => |window| if (is_windows) unreachable else window.vulkanSurfaceExtension(),
        .windows => |window| if (is_windows) window.vulkanSurfaceExtension() else unreachable,
    };
}

pub fn vulkanGetInstanceProcAddr(self: *const WindowManager) !u64 {
    return switch (self.backend) {
        .wayland => if (is_windows) unreachable else error.UnsupportedWindowBackend,
        .windows => |window| if (is_windows)
            try window.vulkanGetInstanceProcAddr()
        else
            unreachable,
    };
}

/// Uses integer handles at this boundary so backend-specific Vulkan C types do
/// not leak into the platform-neutral window manager or renderer.
pub fn createVulkanSurface(
    self: *const WindowManager,
    instance_handle: u64,
    get_instance_proc_addr_handle: u64,
) !u64 {
    return switch (self.backend) {
        .wayland => |window| if (is_windows)
            unreachable
        else
            try window.createVulkanSurface(instance_handle, get_instance_proc_addr_handle),
        .windows => |window| if (is_windows)
            try window.createVulkanSurface(instance_handle, get_instance_proc_addr_handle)
        else
            unreachable,
    };
}

fn convertExtent(resize: anytype) ?Extent {
    const value = resize orelse return null;
    return .{ .width = value.width, .height = value.height };
}
