const std = @import("std");
const mem = std.mem;

const wayland = @import("wayland");
const wl = wayland.client.wl;
const xdg = wayland.client.xdg;

const allocator = std.heap.page_allocator;

const Globals = struct {
    compositor: ?*wl.Compositor,
    wm_base: ?*xdg.WmBase,
};

pub const Window = struct {
    display: *wl.Display,
    surface: *wl.Surface,

    compositor: *wl.Compositor,
    wm_base: *xdg.WmBase,
    xdg_surface: *xdg.Surface,
    xdg_toplevel: *xdg.Toplevel,

    configured: bool,
    running: bool,

    pub fn destroy(self: *Window) void {
        self.xdg_toplevel.destroy();
        self.xdg_surface.destroy();
        self.surface.destroy();
        self.wm_base.destroy();
        self.compositor.destroy();
        self.display.disconnect();
        allocator.destroy(self);
    }
};

pub fn create_window() anyerror!*Window {
    var transferred = false;

    const display = try wl.Display.connect(null);
    errdefer if (!transferred) display.disconnect();

    const registry = try display.getRegistry();
    defer registry.destroy();

    var globals = Globals{
        .compositor = null,
        .wm_base = null,
    };

    registry.setListener(*Globals, registryListener, &globals);
    if (display.roundtrip() != .SUCCESS) return error.RoundtripFailed;

    const compositor = globals.compositor orelse return error.NoWlCompositor;
    errdefer if (!transferred) compositor.destroy();
    const wm_base = globals.wm_base orelse return error.NoXdgWmBase;
    errdefer if (!transferred) wm_base.destroy();

    const surface = try compositor.createSurface();
    errdefer if (!transferred) surface.destroy();
    const xdg_surface = try wm_base.getXdgSurface(surface);
    errdefer if (!transferred) xdg_surface.destroy();
    const xdg_toplevel = try xdg_surface.getToplevel();
    errdefer if (!transferred) xdg_toplevel.destroy();

    const window = try allocator.create(Window);
    errdefer if (transferred) window.destroy() else allocator.destroy(window);

    window.* = .{
        .display = display,
        .surface = surface,
        .compositor = compositor,
        .wm_base = wm_base,
        .xdg_surface = xdg_surface,
        .xdg_toplevel = xdg_toplevel,
        .configured = false,
        .running = true,
    };
    transferred = true;

    xdg_surface.setListener(*Window, xdgSurfaceListener, window);
    xdg_toplevel.setListener(*Window, xdgToplevelListener, window);

    surface.commit();
    while (!window.configured) {
        if (display.dispatch() != .SUCCESS) return error.DispatchFailed;
    }

    return window;
}

fn registryListener(registry: *wl.Registry, event: wl.Registry.Event, globals: *Globals) void {
    switch (event) {
        .global => |global| {
            if (mem.orderZ(u8, global.interface, wl.Compositor.interface.name) == .eq) {
                globals.compositor = registry.bind(global.name, wl.Compositor, 1) catch return;
            } else if (mem.orderZ(u8, global.interface, xdg.WmBase.interface.name) == .eq) {
                globals.wm_base = registry.bind(global.name, xdg.WmBase, 1) catch return;
            }
        },
        .global_remove => {},
    }
}

fn xdgSurfaceListener(xdg_surface: *xdg.Surface, event: xdg.Surface.Event, window: *Window) void {
    switch (event) {
        .configure => |configure| {
            xdg_surface.ackConfigure(configure.serial);
            window.configured = true;
        },
    }
}

fn xdgToplevelListener(_: *xdg.Toplevel, event: xdg.Toplevel.Event, window: *Window) void {
    switch (event) {
        .configure => {},
        .close => window.running = false,
    }
}
