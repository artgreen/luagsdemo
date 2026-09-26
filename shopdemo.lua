-- A shop's nightly restock job: Lua selects policy; C validates and exports.
local inventory = require("inventory")
local policies = dofile("policy.lua")
local stock = inventory.load("stock.csv")
local function money(cents)
    return string.format("%d.%02d", cents // 100, cents % 100)
end
local function display(plan, title, destination)
    local summary = plan:summary()
    print("\n" .. title .. " | Budget $" .. money(summary.budget_cents))
    print("SKU          Action  Want  Buy    Cost")
    for i = 1, summary.count do
        local row = plan:row(i)
        print(string.format("%-12s %-6s %4d %4d %7s",
            row.sku, row.decision, row.requested, row.quantity, money(row.cost_cents)))
        print("  " .. row.reason)
        if row.decision == "DEFER" then print("  C deferred this line: insufficient remaining budget") end
    end
    print("Total $" .. money(summary.total_cents) ..
          " | Remaining $" .. money(summary.remaining_cents))
    plan:write(destination)
    print("Wrote " .. destination)
end
-- Both callbacks finish successfully before either report is written.
local standard = inventory.plan(stock, policies.standard.decide, policies.standard.budget_cents)
local lean = inventory.plan(stock, policies.lean.decide, policies.lean.budget_cents)
print("\nCORNER SHOP RESTOCK PLANNER")
print("Same inventory; two Lua policies; C enforces each budget.")
display(standard, policies.standard.title, "orders.csv")
display(lean, policies.lean.title, "lean.csv")
print("Plans only: inventory is unchanged; no purchases are submitted.")
