const std = @import("std");

const Jueguito = @import("./jueguito.zig");

pub fn main() !void {
    var jueguito = try Jueguito.init();
    defer jueguito.deinit();

    try jueguito.wan();
}
