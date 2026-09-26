local collection = require("collection")
-- Lua indices start at one. Scope exit closes this C-owned allocation;
-- its GC finalizer is also safe after an explicit close.
local nums <close> = collection.new(10)
nums:set(1, mul(4, 10))
nums:set(2, mul(5, 20))
nums:set(3, mul(5, 30))
nums:set(4, 99)
print("Collection size:", nums:size())
for i = 1, nums:size() do print(i, nums:get(i)) end
assert(nums:get(1) == 40 and nums:get(10) == 0)
