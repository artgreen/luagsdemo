local c, s = require("collection"), require("status")
local function fails(f, expected)
    local ok, err = pcall(f)
    assert(not ok and tostring(err):find(expected, 1, true), tostring(err))
end
for _, n in ipairs({-1, 0, 4097, 65536, 0x7fffffff}) do
    fails(function() c.new(n) end, "size must")
end
fails(function() c.new({}) end, "number expected")
fails(function() c.new(1.5) end, "integer")
local a = c.new(3)
assert(a:size() == 3 and a:get(1) == 0)
a:set(1, 70000); a:set(3, -70000)
assert(a:get(1) == 70000 and a:get(3) == -70000)
for _, n in ipairs({-1, 0, 4, 65536, 0x7fffffff}) do
    fails(function() a:get(n) end, "index out of range")
    fails(function() a:set(n, 1) end, "index out of range")
end
fails(function() a:get(1.5) end, "integer")
fails(function() a:set(1, {}) end, "number expected")
a:close(); a:close()
fails(function() a:get(1) end, "closed")
fails(function() a:set(1, 1) end, "closed")
fails(function() a:size() end, "closed")
a = nil; collectgarbage("collect")
local max = c.new(4096); max:set(4096, 99); assert(max:get(4096) == 99); max:close()
local scope
local ok = pcall(function()
    local b <close> = c.new(4)
    scope = b
    error("unwind")
end)
assert(not ok)
fails(function() scope:size() end, "closed")
for i = 1, 200 do
    local b = c.new(16)
    if i % 2 == 0 then b:close() end
end
collectgarbage("collect")
s.setName(string.rep("a", 19)); assert(#s.getName() == 19)
fails(function() s.setName(string.rep("b", 20)) end, "19 bytes")
fails(function() s.setName("a\0b") end, "embedded NULs")
assert(s.getName() == string.rep("a", 19))
s.setName(""); assert(s.getName() == "")
s.setTicks(70000); assert(s.getTicks() == 70000)
s.setTicks(-70000); assert(s.getTicks() == -70000)
fails(function() s.setTicks({}) end, "number expected")
assert(mul(40000, 2) == 80000 and mul(-40000, 2) == -80000)
assert(mul(0x7fffffff, 2) == -2)
fails(function() mul({}, 2) end, "number expected")
print("Binding checks passed")
