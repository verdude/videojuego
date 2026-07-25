const std = @import("std");

const win = @cImport({
    @cDefine("WIN32_LEAN_AND_MEAN", "1");
    @cDefine("NOMINMAX", "1");
    @cDefine("UNICODE", "1");
    @cDefine("_UNICODE", "1");
    @cInclude("windows.h");
    @cDefine("VK_NO_PROTOTYPES", "1");
    @cDefine("VK_USE_PLATFORM_WIN32_KHR", "1");
    @cInclude("vulkan/vulkan.h");
});

const allocator = std.heap.page_allocator;
const MIN_CLIENT_WIDTH: u32 = 960;
const MIN_CLIENT_HEIGHT: u32 = 540;
const WINDOW_STYLE = win.WS_OVERLAPPEDWINDOW;
const WINDOW_EX_STYLE = 0;
const CLASS_NAME = std.unicode.utf8ToUtf16LeStringLiteral("JueguitoWindowClass");
const WINDOW_TITLE = std.unicode.utf8ToUtf16LeStringLiteral("Jueguito");
const VULKAN_LOADER_NAME = std.unicode.utf8ToUtf16LeStringLiteral("vulkan-1.dll");

pub const Window = struct {
    hwnd: win.HWND,
    instance: win.HINSTANCE,
    class_atom: win.ATOM,
    vulkan_library: win.HMODULE,

    running: bool,
    focused: bool,
    minimized: bool,
    interactive_resize: bool,
    resize_pending: bool,
    width: u32,
    height: u32,

    pub const Extent = struct {
        width: u32,
        height: u32,
    };

    pub fn init() !*Window {
        try enablePerMonitorDpiV2();

        const instance = win.GetModuleHandleW(null) orelse return error.GetModuleHandleFailed;
        const vulkan_library = win.LoadLibraryW(VULKAN_LOADER_NAME) orelse
            return error.VulkanLoaderNotFound;
        errdefer _ = win.FreeLibrary(vulkan_library);

        const self = try allocator.create(Window);
        errdefer allocator.destroy(self);

        self.* = .{
            .hwnd = null,
            .instance = instance,
            .class_atom = 0,
            .vulkan_library = vulkan_library,
            .running = true,
            .focused = true,
            .minimized = false,
            .interactive_resize = false,
            .resize_pending = false,
            .width = MIN_CLIENT_WIDTH,
            .height = MIN_CLIENT_HEIGHT,
        };

        const window_class = win.WNDCLASSEXW{
            .cbSize = @sizeOf(win.WNDCLASSEXW),
            .style = win.CS_HREDRAW | win.CS_VREDRAW | win.CS_OWNDC,
            .lpfnWndProc = windowProc,
            .cbClsExtra = 0,
            .cbWndExtra = 0,
            .hInstance = instance,
            .hIcon = null,
            .hCursor = win.LoadCursorW(null, @ptrFromInt(32512)),
            .hbrBackground = null,
            .lpszMenuName = null,
            .lpszClassName = CLASS_NAME,
            .hIconSm = null,
        };
        self.class_atom = win.RegisterClassExW(&window_class);
        if (self.class_atom == 0) return error.RegisterWindowClassFailed;
        errdefer _ = win.UnregisterClassW(CLASS_NAME, instance);

        var window_rect = win.RECT{
            .left = 0,
            .top = 0,
            .right = MIN_CLIENT_WIDTH,
            .bottom = MIN_CLIENT_HEIGHT,
        };
        const dpi = win.GetDpiForSystem();
        if (win.AdjustWindowRectExForDpi(
            &window_rect,
            WINDOW_STYLE,
            win.FALSE,
            WINDOW_EX_STYLE,
            dpi,
        ) == 0) {
            if (win.AdjustWindowRectEx(
                &window_rect,
                WINDOW_STYLE,
                win.FALSE,
                WINDOW_EX_STYLE,
            ) == 0) return error.AdjustWindowRectFailed;
        }

        const hwnd = win.CreateWindowExW(
            WINDOW_EX_STYLE,
            CLASS_NAME,
            WINDOW_TITLE,
            WINDOW_STYLE,
            win.CW_USEDEFAULT,
            win.CW_USEDEFAULT,
            window_rect.right - window_rect.left,
            window_rect.bottom - window_rect.top,
            null,
            null,
            instance,
            self,
        ) orelse return error.CreateWindowFailed;
        self.hwnd = hwnd;
        errdefer {
            _ = win.DestroyWindow(hwnd);
            self.hwnd = null;
        }

        self.updateClientExtent();
        _ = win.ShowWindow(hwnd, win.SW_SHOW);
        _ = win.UpdateWindow(hwnd);
        return self;
    }

    /// Drains the thread's Win32 message queue without blocking.
    pub fn pollEvents(self: *Window) !void {
        var message: win.MSG = undefined;
        while (win.PeekMessageW(&message, null, 0, 0, win.PM_REMOVE) != 0) {
            if (message.message == win.WM_QUIT) {
                self.running = false;
                continue;
            }
            _ = win.TranslateMessage(&message);
            _ = win.DispatchMessageW(&message);
        }
    }

    /// Returns a resize only after the callback has marked the swapchain stale.
    pub fn takeResize(self: *Window) ?Extent {
        if (!self.resize_pending or self.minimized or self.interactive_resize or
            self.width == 0 or self.height == 0)
        {
            return null;
        }

        self.resize_pending = false;
        return .{ .width = self.width, .height = self.height };
    }

    pub fn canRender(self: *const Window) bool {
        return self.running and !self.minimized and !self.interactive_resize and
            self.width != 0 and self.height != 0;
    }

    pub fn vulkanSurfaceExtension(_: *const Window) [*:0]const u8 {
        return "VK_KHR_win32_surface";
    }

    pub fn vulkanGetInstanceProcAddr(self: *const Window) !u64 {
        const address = win.GetProcAddress(self.vulkan_library, "vkGetInstanceProcAddr") orelse
            return error.VulkanLoaderEntryPointNotFound;
        return @intFromPtr(address);
    }

    /// Creates the Win32 Vulkan surface while keeping HWND/HINSTANCE private here.
    pub fn createVulkanSurface(
        self: *const Window,
        instance_handle: u64,
        get_instance_proc_addr_handle: u64,
    ) !u64 {
        const instance = vulkanHandleFromU64(win.VkInstance, instance_handle);
        const get_instance_proc_addr = vulkanHandleFromU64(
            requiredProcType(win.PFN_vkGetInstanceProcAddr),
            get_instance_proc_addr_handle,
        );
        const raw_create_surface = get_instance_proc_addr(
            instance,
            "vkCreateWin32SurfaceKHR",
        ) orelse return error.VulkanSurfaceEntryPointNotFound;
        const create_surface: requiredProcType(win.PFN_vkCreateWin32SurfaceKHR) =
            @ptrCast(raw_create_surface);
        const create_info = win.VkWin32SurfaceCreateInfoKHR{
            .sType = win.VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .pNext = null,
            .flags = 0,
            .hinstance = self.instance,
            .hwnd = self.hwnd,
        };
        var surface: win.VkSurfaceKHR = std.mem.zeroes(win.VkSurfaceKHR);
        if (create_surface(instance, &create_info, null, &surface) != win.VK_SUCCESS) {
            return error.VulkanSurfaceCreationFailed;
        }
        return vulkanHandleToU64(surface);
    }

    pub fn deinit(self: *Window) void {
        if (self.hwnd) |hwnd| {
            _ = win.DestroyWindow(hwnd);
            self.hwnd = null;
        }
        if (self.class_atom != 0) {
            _ = win.UnregisterClassW(CLASS_NAME, self.instance);
            self.class_atom = 0;
        }
        _ = win.FreeLibrary(self.vulkan_library);
        allocator.destroy(self);
    }

    fn updateClientExtent(self: *Window) void {
        const hwnd = self.hwnd orelse return;
        var rect: win.RECT = undefined;
        if (win.GetClientRect(hwnd, &rect) == 0) return;

        self.width = @intCast(@max(rect.right - rect.left, 0));
        self.height = @intCast(@max(rect.bottom - rect.top, 0));
    }

    fn markResize(self: *Window, width: u32, height: u32, minimized: bool) void {
        self.width = width;
        self.height = height;
        self.minimized = minimized or width == 0 or height == 0;
        self.resize_pending = true;
    }

    fn handleMessage(
        self: *Window,
        hwnd: win.HWND,
        message: win.UINT,
        w_param: win.WPARAM,
        l_param: win.LPARAM,
    ) ?win.LRESULT {
        switch (message) {
            win.WM_CLOSE => {
                // Native destruction is deferred until after Vulkan teardown.
                self.running = false;
                return 0;
            },
            win.WM_DESTROY => {
                self.running = false;
                self.hwnd = null;
                return 0;
            },
            win.WM_SIZE => {
                const packed_size: usize = @bitCast(l_param);
                const width: u32 = @intCast(packed_size & 0xffff);
                const height: u32 = @intCast((packed_size >> 16) & 0xffff);
                self.markResize(width, height, w_param == win.SIZE_MINIMIZED);
                return 0;
            },
            win.WM_DPICHANGED => {
                const suggested: *const win.RECT = @ptrFromInt(@as(usize, @bitCast(l_param)));
                _ = win.SetWindowPos(
                    hwnd,
                    null,
                    suggested.left,
                    suggested.top,
                    suggested.right - suggested.left,
                    suggested.bottom - suggested.top,
                    win.SWP_NOACTIVATE | win.SWP_NOZORDER,
                );
                self.updateClientExtent();
                self.resize_pending = true;
                return 0;
            },
            win.WM_SETFOCUS => {
                self.focused = true;
                return 0;
            },
            win.WM_KILLFOCUS => {
                self.focused = false;
                return 0;
            },
            win.WM_ENTERSIZEMOVE => {
                self.interactive_resize = true;
                return 0;
            },
            win.WM_EXITSIZEMOVE => {
                self.interactive_resize = false;
                self.updateClientExtent();
                self.resize_pending = true;
                return 0;
            },
            win.WM_GETMINMAXINFO => {
                const info: *win.MINMAXINFO = @ptrFromInt(@as(usize, @bitCast(l_param)));
                var rect = win.RECT{
                    .left = 0,
                    .top = 0,
                    .right = MIN_CLIENT_WIDTH,
                    .bottom = MIN_CLIENT_HEIGHT,
                };
                const dpi = win.GetDpiForWindow(hwnd);
                _ = win.AdjustWindowRectExForDpi(
                    &rect,
                    WINDOW_STYLE,
                    win.FALSE,
                    WINDOW_EX_STYLE,
                    dpi,
                );
                info.ptMinTrackSize.x = rect.right - rect.left;
                info.ptMinTrackSize.y = rect.bottom - rect.top;
                return 0;
            },
            win.WM_NCDESTROY => {
                _ = win.SetWindowLongPtrW(hwnd, win.GWLP_USERDATA, 0);
                return null;
            },
            else => return null,
        }
    }
};

fn requiredProcType(comptime Proc: type) type {
    return @typeInfo(Proc).optional.child;
}

fn vulkanHandleFromU64(comptime Handle: type, value: u64) Handle {
    return switch (@typeInfo(Handle)) {
        .optional, .pointer => @ptrFromInt(@as(usize, @intCast(value))),
        .int => @intCast(value),
        else => @compileError("unsupported Vulkan handle representation"),
    };
}

fn vulkanHandleToU64(handle: anytype) u64 {
    return switch (@typeInfo(@TypeOf(handle))) {
        .optional, .pointer => @intFromPtr(handle),
        .int => @intCast(handle),
        else => @compileError("unsupported Vulkan handle representation"),
    };
}

fn enablePerMonitorDpiV2() !void {
    if (win.SetProcessDpiAwarenessContext(win.DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) != 0) {
        return;
    }

    // ERROR_ACCESS_DENIED means awareness was already fixed (usually by a
    // manifest). Accept it only when the effective context is already V2.
    if (win.GetLastError() == win.ERROR_ACCESS_DENIED and
        win.AreDpiAwarenessContextsEqual(
            win.GetThreadDpiAwarenessContext(),
            win.DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2,
        ) != 0)
    {
        return;
    }
    return error.SetDpiAwarenessFailed;
}

fn windowProc(
    hwnd: win.HWND,
    message: win.UINT,
    w_param: win.WPARAM,
    l_param: win.LPARAM,
) callconv(.winapi) win.LRESULT {
    if (message == win.WM_NCCREATE) {
        const create: *const win.CREATESTRUCTW = @ptrFromInt(@as(usize, @bitCast(l_param)));
        const self: *Window = @ptrCast(@alignCast(create.lpCreateParams));
        _ = win.SetWindowLongPtrW(hwnd, win.GWLP_USERDATA, @intCast(@intFromPtr(self)));
        self.hwnd = hwnd;
    }

    const user_data = win.GetWindowLongPtrW(hwnd, win.GWLP_USERDATA);
    if (user_data != 0) {
        const self: *Window = @ptrFromInt(@as(usize, @intCast(user_data)));
        if (self.handleMessage(hwnd, message, w_param, l_param)) |result| return result;
    }

    return win.DefWindowProcW(hwnd, message, w_param, l_param);
}
