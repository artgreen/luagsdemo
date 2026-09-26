# A shop restocking planner in C and Lua

A small shop wants to replenish fast-selling products without exceeding its
weekly purchasing budget. Staff can change the stocking policy as conditions
change. This example makes that policy a Lua script while C owns the inventory,
validation, budget accounting, and report format.

Run `make run`, or run only this application with:

```sh
export GOLDEN_GATE=/absolute/path/to/orca-sdk-2.2.1
iix --memcheck build/luademo --script shopdemo.lua
```

On the IIgs, use `luademo --script shopdemo.lua` from the extracted package's
writable directory. All filenames resolve from that directory.

## Follow one item through the application

1. C reads `stock.csv`. Coffee has 4 units on hand, no incoming stock, sales of
   12 units per week, a supplier pack size of 6, and a unit cost of 650 cents.
2. `shopdemo.lua` loads `policy.lua` and asks C to construct a plan. C calls the
   Lua policy once per item, passing a table containing a copy of the C record.
3. The standard Lua rule sees less than one week's supply. Its two-week target
   is 24 units. The shortfall is 20, rounded up to four supplier packs: 24 units.
4. C validates that the proposal is an integer, within bounds, and a multiple
   of the supplier's pack size. It computes 24 * 650 = 15600 cents using the
   original C record and checks the remaining budget.
5. Lua displays the accepted plan. C writes its validated records to CSV.

The same data under the lean policy requests two packs of coffee instead of
four. Changing the Lua rule changes the recommendation without recompiling.

| Policy | Budget | Coffee ordered | Filters ordered | Total | Remaining |
| --- | ---: | ---: | ---: | ---: | ---: |
| Two-week target | $200.00 | 24 | 20 | $192.00 | $8.00 |
| One-week target | $100.00 | 12 | 10 | $96.00 | $4.00 |

Tea and syrup are deferred because their whole recommendations do not fit the
remaining budget. Mugs already have enough stock, and cocoa has a delivery on
the way. A `HOLD` means no order was requested; a `DEFER` means the rule requested
an order that C could not fund. Deferred rows retain the requested quantity and
reason, with accepted quantity and cost set to zero.

## Change a policy

`policy.lua` returns two named policies. Each has a title, budget in cents, and
`decide(item)` callback returning `quantity, reason`.

```lua
-- One pack whenever available stock falls below weekly sales.
local function one_pack(item)
    if item.on_hand + item.on_order < item.weekly_sales then
        return item.pack, "Below weekly sales; order one pack"
    end
    return 0, "Enough stock"
end
```

Use that function as a policy's `decide` value and rerun the same binary. You
can also adjust the budgets or target weeks. Pack rounding is deliberately
visible in Lua; C independently rejects invalid quantities.

The planner processes items in CSV order. An entire recommendation is accepted
or deferred; it does not shrink orders to spend the remaining budget. Later
items may still fit. This is a deterministic planning example, not a purchasing
optimizer. Put higher-priority inventory earlier if priority should affect the
result. Currency is always integer cents to avoid floating-point rounding.

## C/Lua boundary

| File | Responsibility |
| --- | --- |
| `inventory.c` / `inventory.h` | Import, C-owned records, callback invocation, validation, budgets, CSV export |
| `stock.csv` | Six sample inventory records |
| `policy.lua` | Editable standard and lean replenishment rules |
| `shopdemo.lua` | Orchestrate both plans, format output, request report exports |
| `main.c` | Initialize Lua and register the native `inventory` module |

The module's public API is:

```lua
local inventory = require("inventory")
local stock = inventory.load("stock.csv")
local plan = inventory.plan(stock, decide, budget_cents)
local summary = plan:summary()
local row = plan:row(1)
plan:write("orders.csv")
```

`stock` and `plan` are Lua userdata containing C-owned values, reclaimed by Lua's
collector. No file handles or external heap pointers outlive an operation.
Each callback gets a fresh table. Changing its price or pack size cannot alter
the C record used for validation. `row()` and `summary()` also return copies;
editing them does not change the report. `row()` uses one-based indexing.

Summary fields are `count`, `budget_cents`, `total_cents`, and `remaining_cents`.
Rows contain the seven inventory fields plus `requested`, `quantity`,
`cost_cents`, `decision`, and `reason`. C rejects invalid callback results and
propagates callback errors to the host's protected Lua call. No partial plan is
returned. Both example plans are computed successfully before exporting either.

The scripts are trusted application code with standard Lua libraries. This
boundary prevents accidental data corruption through this module; it is not a
sandbox for hostile scripts.

## Inventory format and limits

The exact header is:

```csv
sku,name,on_hand,on_order,weekly_sales,pack,unit_cents
COFFEE,Ground coffee,4,0,12,6,650
```

The importer accepts LF, CRLF, and native ProDOS CR line endings. It intentionally
supports a simple, unquoted format: seven comma-separated fields, printable
ASCII, no commas or quotes within names, and no blank records. Numeric fields
are unsigned decimal integers without surrounding spaces.

- Between 1 and 32 unique SKUs. SKUs are 1–12 uppercase letters, digits, hyphens,
  or periods; names are 1–31 bytes.
- On-hand, incoming, and weekly-sales quantities are 0–10000.
- Pack sizes are 1–1000; unit costs are 1–100000 cents.
- Policy quantities are 0–10000 and must be whole packs.
- Policy reasons are 1–63 printable ASCII bytes.
- Budgets are 0–100000000 cents. Validated multiplication fits a signed 32-bit
  value, and accepted totals cannot exceed the budget.

Malformed rows, duplicate SKUs, overlong records, and arithmetic limits produce
errors rather than truncating data. The input file is opened read-only.

## Reports and scope

`orders.csv` and `lean.csv` contain every inventory row, including holds and
deferrals. Output uses CR line endings, integer-cent costs, and standard CSV
quoting, including doubled quotes inside text. They can be reviewed or imported
into a spreadsheet. They are order recommendations, not purchase transactions.

Running the demo replaces these two generated reports. Each write is a separate
operation; use filenames reserved for reports. On a write failure, C closes the
file, removes the partial output, and reports the error. The demo does not
provide atomic replacement, a database, concurrent writers, stock updates,
supplier integration, or a transaction spanning both report files.

See [validation](VALIDATION.md) for executable tests and hardware acceptance.
