local inv = require("inventory")
local policies = dofile("policy.lua")
local stock = inv.load("stock.csv")
local function fails(f, text)
    local ok, err = pcall(f)
    assert(not ok and tostring(err):find(text, 1, true), tostring(err))
end
local standard = inv.plan(stock, policies.standard.decide, 20000)
local lean = inv.plan(stock, policies.lean.decide, 10000)
assert(standard:summary().total_cents == 19200)
assert(lean:summary().total_cents == 9600)
assert(standard:row(1).quantity == 24 and lean:row(1).quantity == 12)
assert(standard:row(2).decision == "DEFER" and standard:row(2).requested == 16)
assert(standard:row(5).decision == "HOLD") -- Deliveries already cover sales.
assert(inv.plan(stock, policies.standard.decide, 0):summary().total_cents == 0)
assert(inv.plan(stock, policies.standard.decide, 15600):row(1).quantity == 24)
assert(inv.plan(stock, policies.standard.decide, 15599):row(1).decision == "DEFER")
for _, budget in ipairs({-1, 100000001}) do
    fails(function() inv.plan(stock, policies.standard.decide, budget) end, "budget must")
end
fails(function() inv.plan(stock, policies.standard.decide, 1.5) end, "integer")
for _, qty in ipairs({-6, 1, 10002}) do
    fails(function() inv.plan(stock, function() return qty, "bad" end, 20000) end, "whole packs")
end
for _, qty in ipairs({1.5, "6", false}) do
    fails(function() inv.plan(stock, function() return qty, "bad" end, 20000) end, "must be an integer")
end
fails(function() inv.plan(stock, function() return 6 end, 20000) end, "must be a string")
fails(function() inv.plan(stock, function() return 6, "" end, 20000) end, "1..63 bytes")
fails(function() inv.plan(stock, function() return 6, string.rep("x",64) end, 20000) end, "1..63 bytes")
fails(function() inv.plan(stock, function() return 6, "bad\nreason" end, 20000) end, "printable ASCII")
fails(function() inv.plan(stock, function() return 6, "bad\0reason" end, 20000) end, "printable ASCII")
-- A failure after several successful callbacks must not publish a partial plan.
fails(function()
    inv.plan(stock, function(item)
        if item.sku == "FILTER" then error("policy stopped") end
        return item.pack, "test"
    end, 20000)
end, "policy stopped")
assert(inv.plan(stock, policies.standard.decide, 20000):summary().total_cents == 19200)
local independent = inv.plan(stock, function(item)
    local pack = item.pack
    item.unit_cents = 1; item.pack = 1; item.sku = "CHANGED"
    return pack, 'A "quoted", reason'
end, 20000)
assert(independent:row(1).unit_cents == 650 and independent:row(1).cost_cents == 3900)
assert(independent:row(1).sku == "COFFEE")
local copy = independent:row(1); copy.quantity = 999; copy.cost_cents = 0
assert(independent:row(1).quantity == 6)
local summary = independent:summary(); summary.total_cents = 0
assert(independent:summary().total_cents > 0)
for _, i in ipairs({0, 7, 65536}) do fails(function() standard:row(i) end, "row out of range") end
fails(function() standard:write("build/no-such-dir/report.csv") end, "cannot write report")
fails(function() inv.load("stock.csv\0other") end, "invalid file path")
independent:write("build/quoted.csv")
for i = 1, 30 do inv.plan(stock, policies.standard.decide, 20000) end
collectgarbage("collect")
print("Inventory and policy checks passed")
