-- Edit these rules without recompiling the C application.
-- Currency is integer cents throughout; no floating-point rounding.
local function replenish(item, weeks)
    local available = item.on_hand + item.on_order
    if available >= item.weekly_sales then
        return 0, "Enough stock including deliveries"
    end
    local shortfall = math.max(0, item.weekly_sales * weeks - available)
    local packs = (shortfall + item.pack - 1) // item.pack
    return packs * item.pack, "Below one week; restore target stock"
end
return {
    standard = {
        title = "Two-week stock target",
        budget_cents = 20000,
        decide = function(item) return replenish(item, 2) end,
    },
    lean = {
        title = "One-week stock target",
        budget_cents = 10000,
        decide = function(item) return replenish(item, 1) end,
    },
}
