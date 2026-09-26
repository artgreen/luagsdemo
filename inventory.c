/* C owns inventory, validates Lua's proposals, and writes the final report. */
#pragma memorymodel 1
#pragma noroot
#include <stdio.h>
#include <string.h>
#include "lua.h"
#include "lauxlib.h"
#include "inventory.h"
#define STOCK "luagsdemo.stock"
#define PLAN "luagsdemo.plan"
#define MAX_ITEMS 32
#define MAX_QTY 10000L
#define MAX_BUDGET 100000000L

typedef struct {
    char sku[13], name[32];
    long on_hand, on_order, weekly_sales, pack, unit_cents;
} Item;
typedef struct { int count; Item items[MAX_ITEMS]; } Stock;
typedef struct {
    Item item;
    long requested, quantity, cost;
    char reason[64];
} Order;
typedef struct { int count; long budget, total; Order rows[MAX_ITEMS]; } Plan;

/* Read LF, CRLF, or native ProDOS CR without silently truncating a record. */
static int read_line(FILE *f, char *line, size_t capacity) {
    int c, next;
    size_t n = 0;
    while ((c = fgetc(f)) != EOF) {
        if (c == '\r' || c == '\n') {
            if (c == '\r') { next = fgetc(f); if (next != '\n' && next != EOF) ungetc(next, f); }
            line[n] = 0;
            return 1;
        }
        if (c < 32 || c > 126 || n + 1 >= capacity) return -1;
        line[n++] = (char)c;
    }
    line[n] = 0;
    return ferror(f) ? -1 : (n ? 1 : 0);
}
static int number(const char *s, long limit, long *out) {
    long n = 0;
    if (!*s) return 0;
    for (; *s; ++s) {
        if (*s < '0' || *s > '9' || n > (limit - (*s - '0')) / 10) return 0;
        n = n * 10 + (*s - '0');
    }
    *out = n;
    return 1;
}
static int parse_item(char *line, Item *item) {
    char *fields[7], *p;
    int i = 1;
    fields[0] = line;
    for (p = line; *p; ++p) {
        if (*p == '"') return 0; /* Deliberately a simple, unquoted import format. */
        if (*p == ',') { if (i == 7) return 0; *p = 0; fields[i++] = p + 1; }
    }
    if (i != 7 || !fields[0][0] || strlen(fields[0]) > 12 ||
        !fields[1][0] || strlen(fields[1]) > 31) return 0;
    for (p = fields[0]; *p; ++p)
        if (!((*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '-' || *p == '.')) return 0;
    strcpy(item->sku, fields[0]); strcpy(item->name, fields[1]);
    return number(fields[2], MAX_QTY, &item->on_hand) &&
           number(fields[3], MAX_QTY, &item->on_order) &&
           number(fields[4], MAX_QTY, &item->weekly_sales) &&
           number(fields[5], 1000, &item->pack) && item->pack > 0 &&
           number(fields[6], 100000, &item->unit_cents) && item->unit_cents > 0;
}
static const char *path_arg(lua_State *L, int index) {
    size_t n;
    const char *s = luaL_checklstring(L, index, &n);
    luaL_argcheck(L, n > 0 && n <= 255 && !memchr(s, 0, n), index, "invalid file path");
    return s;
}
static int load_stock(lua_State *L) {
    const char *path = path_arg(L, 1);
    Stock *stock = lua_newuserdatauv(L, sizeof(Stock), 0);
    FILE *f;
    char line[192];
    int status, row = 1, j;
    const char *error = NULL;
    stock->count = 0;
    luaL_setmetatable(L, STOCK);
    f = fopen(path, "rb");
    if (!f) return luaL_error(L, "cannot open inventory: %s", path);
    /* No Lua calls while a FILE is open: a Lua error must not leak it. */
    status = read_line(f, line, sizeof(line));
    if (status != 1 || strcmp(line, "sku,name,on_hand,on_order,weekly_sales,pack,unit_cents"))
        error = "invalid header";
    while (!error && (status = read_line(f, line, sizeof(line))) != 0) {
        ++row;
        if (status < 0) { error = "invalid or overlong line"; break; }
        if (stock->count == MAX_ITEMS) { error = "more than 32 items"; break; }
        if (!parse_item(line, &stock->items[stock->count])) { error = "invalid item fields"; break; }
        for (j = 0; j < stock->count; ++j)
            if (!strcmp(stock->items[j].sku, stock->items[stock->count].sku)) error = "duplicate SKU";
        if (!error) ++stock->count;
    }
    if (fclose(f) && !error) error = "read/close error";
    if (error) return luaL_error(L, "inventory line %d: %s", row, error);
    if (!stock->count) return luaL_error(L, "inventory is empty");
    return 1;
}
static void integer_field(lua_State *L, const char *name, long value) {
    lua_pushinteger(L, (lua_Integer)value); lua_setfield(L, -2, name);
}
static void string_field(lua_State *L, const char *name, const char *value) {
    lua_pushstring(L, value); lua_setfield(L, -2, name);
}
static void push_item(lua_State *L, const Item *item) {
    lua_createtable(L, 0, 7);
    string_field(L, "sku", item->sku); string_field(L, "name", item->name);
    integer_field(L, "on_hand", item->on_hand); integer_field(L, "on_order", item->on_order);
    integer_field(L, "weekly_sales", item->weekly_sales); integer_field(L, "pack", item->pack);
    integer_field(L, "unit_cents", item->unit_cents);
}
static int make_plan(lua_State *L) {
    Stock *stock = luaL_checkudata(L, 1, STOCK);
    lua_Integer budget = luaL_checkinteger(L, 3), qty;
    Plan *plan;
    Order *row;
    const char *reason;
    size_t length, j;
    int i;
    luaL_checktype(L, 2, LUA_TFUNCTION);
    luaL_argcheck(L, budget >= 0 && budget <= MAX_BUDGET, 3, "budget must be 0..100000000 cents");
    plan = lua_newuserdatauv(L, sizeof(Plan), 0);
    plan->count = 0; plan->budget = budget; plan->total = 0;
    luaL_setmetatable(L, PLAN);
    /* This userdata is private until every callback succeeds. No file writes,
     * stock changes, or purchase side effects happen while planning. */
    for (i = 0; i < stock->count; ++i) {
        row = &plan->rows[i]; row->item = stock->items[i];
        lua_pushvalue(L, 2); push_item(L, &row->item);
        lua_call(L, 1, 2); /* The host's lg_run_file boundary catches errors. */
        if (!lua_isinteger(L, -2)) return luaL_error(L, "%s: policy quantity must be an integer", row->item.sku);
        qty = lua_tointeger(L, -2);
        if (qty < 0 || qty > MAX_QTY || qty % row->item.pack)
            return luaL_error(L, "%s: policy quantity must be 0..10000 in whole packs", row->item.sku);
        if (lua_type(L, -1) != LUA_TSTRING) return luaL_error(L, "%s: policy reason must be a string", row->item.sku);
        reason = lua_tolstring(L, -1, &length);
        if (!length || length >= sizeof(row->reason)) return luaL_error(L, "%s: policy reason must be 1..63 bytes", row->item.sku);
        for (j = 0; j < length; ++j)
            if ((unsigned char)reason[j] < 32 || (unsigned char)reason[j] > 126)
                return luaL_error(L, "%s: policy reason must be printable ASCII", row->item.sku);
        memcpy(row->reason, reason, length); row->reason[length] = 0;
        lua_pop(L, 2);
        row->requested = qty;
        /* Validated limits make the multiplication fit signed 32-bit long. */
        row->cost = (long)qty * row->item.unit_cents;
        row->quantity = row->cost <= plan->budget - plan->total ? (long)qty : 0;
        if (!row->quantity) row->cost = 0;
        plan->total += row->cost;
        ++plan->count;
    }
    return 1;
}
static const char *decision(const Order *row) {
    return row->quantity ? "ORDER" : (row->requested ? "DEFER" : "HOLD");
}
static int plan_summary(lua_State *L) {
    Plan *p = luaL_checkudata(L, 1, PLAN);
    lua_createtable(L, 0, 4);
    integer_field(L, "count", p->count); integer_field(L, "budget_cents", p->budget);
    integer_field(L, "total_cents", p->total); integer_field(L, "remaining_cents", p->budget - p->total);
    return 1;
}
static int plan_row(lua_State *L) {
    Plan *p = luaL_checkudata(L, 1, PLAN);
    lua_Integer index = luaL_checkinteger(L, 2);
    Order *row;
    luaL_argcheck(L, index >= 1 && index <= p->count, 2, "row out of range");
    row = &p->rows[(int)index - 1];
    push_item(L, &row->item);
    integer_field(L, "requested", row->requested); integer_field(L, "quantity", row->quantity);
    integer_field(L, "cost_cents", row->cost); string_field(L, "decision", decision(row));
    string_field(L, "reason", row->reason);
    return 1;
}
static int csv_string(FILE *f, const char *s) {
    if (fputc('"', f) == EOF) return 0;
    for (; *s; ++s) {
        if (*s == '"' && fputc('"', f) == EOF) return 0;
        if (fputc(*s, f) == EOF) return 0;
    }
    return fputc('"', f) != EOF;
}
static int write_plan(lua_State *L) {
    Plan *p = luaL_checkudata(L, 1, PLAN);
    const char *path = path_arg(L, 2);
    FILE *f = fopen(path, "wb");
    Order *row;
    int i, ok;
    if (!f) return luaL_error(L, "cannot write report: %s", path);
    ok = fprintf(f, "sku,name,decision,requested,quantity,unit_cents,cost_cents,reason\r") >= 0;
    for (i = 0; ok && i < p->count; ++i) {
        row = &p->rows[i];
        ok = csv_string(f, row->item.sku) && fputc(',', f) != EOF &&
             csv_string(f, row->item.name) &&
             fprintf(f, ",%s,%ld,%ld,%ld,%ld,", decision(row), row->requested,
                     row->quantity, row->item.unit_cents, row->cost) >= 0 &&
             csv_string(f, row->reason) && fputc('\r', f) != EOF;
    }
    if (fclose(f)) ok = 0;
    if (!ok) { remove(path); return luaL_error(L, "report write failed: %s", path); }
    return 0;
}
static const luaL_Reg plan_methods[] = {
    {"summary", plan_summary}, {"row", plan_row}, {"write", write_plan}, {NULL, NULL}
};
static const luaL_Reg functions[] = {{"load", load_stock}, {"plan", make_plan}, {NULL, NULL}};
int luaopen_inventory(lua_State *L) {
    luaL_newmetatable(L, STOCK);
    lua_pushliteral(L, "inventory"); lua_setfield(L, -2, "__metatable"); lua_pop(L, 1);
    luaL_newmetatable(L, PLAN); luaL_setfuncs(L, plan_methods, 0);
    lua_pushvalue(L, -1); lua_setfield(L, -2, "__index");
    lua_pushliteral(L, "order plan"); lua_setfield(L, -2, "__metatable"); lua_pop(L, 1);
    luaL_newlib(L, functions);
    return 1;
}
