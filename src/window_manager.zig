const Wayland = @import("./wayland.zig").Window;

pub const WindowManager = @This();

pub const Backend = union(enum) {
    wayland: *Wayland,
};

pub const BackendType = @typeInfo(Backend).@"union".tag_type.?;
pub const Extent = struct {
    width: u32,
    height: u32,
};

backend: Backend,

pub fn init(backend_type: BackendType) !WindowManager {
    return .{
        .backend = switch (backend_type) {
            .wayland => .{ .wayland = try Wayland.init() },
        },
    };
}

pub fn deinit(self: *WindowManager) void {
    switch (self.backend) {
        .wayland => |window| window.deinit(),
    }
}

pub fn pollEvents(self: *WindowManager) !void {
    switch (self.backend) {
        .wayland => |window| try window.pollEvents(),
    }
}

pub fn takeResize(self: *WindowManager) ?Extent {
    return switch (self.backend) {
        .wayland => |window| if (window.takeResize()) |resize| .{
            .width = resize.width,
            .height = resize.height,
        } else null,
    };
}

pub fn isRunning(self: *const WindowManager) bool {
    return switch (self.backend) {
        .wayland => |window| window.running,
    };
}

pub fn extent(self: *const WindowManager) Extent {
    return switch (self.backend) {
        .wayland => |window| .{
            .width = @intCast(window.width),
            .height = @intCast(window.height),
        },
    };
}
