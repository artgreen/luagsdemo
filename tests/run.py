"""Run the actual IIgs binaries; no desktop Lua substitute."""
import csv
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
iix, exe = sys.argv[1:]
results = []
stock_before = hashlib.sha256((ROOT / 'stock.csv').read_bytes()).hexdigest()
def check(name, args, ok, expected, executable=exe):
    p = subprocess.run([iix, '--memcheck', executable, *args], cwd=ROOT,
                       stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=60)
    output = p.stdout.decode(errors='replace')
    passed = (p.returncode == 0) == ok and all(s in output for s in expected)
    # GoldenGate errors must not masquerade as expected application failures.
    passed = passed and not any(s in output.lower() for s in ('invalid read', 'invalid write', 'uninitialized', 'unimplemented', 'halted', 'memory error'))
    results.append({'name': name, 'passed': passed, 'exit_code': p.returncode, 'output': output})
    print(('PASS ' if passed else 'FAIL ') + name, flush=True)
    if not passed: print(output)

check('default demo', [], True, ['CORNER SHOP RESTOCK PLANNER', 'Total $192.00 | Remaining $8.00', 'Total $96.00 | Remaining $4.00', 'Collection size:', 'C -> Lua -> C: 21 -> 42', 'C sees status: 42, New status', 'Demo completed'])
def report_check(name, path, quantities, total):
    try:
        with path.open(newline='') as f:
            rows = list(csv.DictReader(f))
        passed = ([r['sku'] for r in rows] == ['COFFEE', 'TEA', 'FILTER', 'MUG', 'COCOA', 'SYRUP']
                  and [int(r['quantity']) for r in rows] == quantities
                  and sum(int(r['cost_cents']) for r in rows) == total
                  and all(int(r['quantity']) * int(r['unit_cents']) == int(r['cost_cents']) for r in rows))
    except (OSError, ValueError, KeyError):
        passed = False
    results.append({'name': name, 'passed': passed})
    print(('PASS ' if passed else 'FAIL ') + name)
report_check('standard CSV report', ROOT / 'orders.csv', [24, 0, 20, 0, 0, 0], 19200)
report_check('lean CSV report', ROOT / 'lean.csv', [12, 0, 10, 0, 0, 0], 9600)
check('inventory and policy boundaries', ['--script', 'tests/inventory.lua'], True, ['Inventory and policy checks passed'])
try:
    with (ROOT / 'build/quoted.csv').open(newline='') as f:
        quoted = list(csv.DictReader(f))
    quoted_ok = len(quoted) == 6 and all(r['reason'] == 'A "quoted", reason' for r in quoted)
except (OSError, KeyError):
    quoted_ok = False
results.append({'name': 'CSV quoting', 'passed': quoted_ok})
print(('PASS ' if quoted_ok else 'FAIL ') + 'CSV quoting')
check('binding bounds, lifetimes, and integer widths', ['--script', 'tests/bindings.lua'], True, ['Binding checks passed'])
check('C host error recovery', [], True, ['Host checks passed'], str(ROOT / 'build/hosttest'))
check('missing script', ['--script', 'tests/does-not-exist.lua'], False, ['Lua error:'])
check('missing config', ['tests/does-not-exist.lua'], False, ['Lua error:'])
check('invalid arguments', ['a', 'b'], False, ['Usage:'])
cases = [
    ('empty config', 'scripts={}', True, ['0 scripts', 'Demo completed']),
    ('missing array', '', False, ['Invalid scripts']),
    ('global lookup error', 'setmetatable(_G,{__index=function() error("lookup error") end})', False, ['Lua error:', 'lookup error']),
    ('non-table array', 'scripts=42', False, ['Invalid scripts']),
    ('bad item type', 'scripts={1}', False, ['Invalid scripts']),
    ('empty path', 'scripts={""}', False, ['Invalid scripts']),
    ('long path', 'scripts={string.rep("a",64)}', False, ['Invalid scripts']),
    ('embedded NUL', 'scripts={"a\\0b"}', False, ['Invalid scripts']),
    ('too many scripts', 'scripts={};for i=1,9 do scripts[i]="coltest.lua" end', False, ['Invalid scripts']),
    ('eight scripts', 'scripts={};for i=1,8 do scripts[i]="coltest.lua" end', True, ['8 scripts', 'Demo completed']),
    ('array hole', 'scripts={[1]="coltest.lua",[3]="coltest.lua"}', False, ['Invalid scripts']),
    ('extra array key', 'scripts={"coltest.lua",named="stattest.lua"}', False, ['Invalid scripts']),
    ('configured missing script', 'scripts={"missing.lua"}', False, ['Lua error:']),
    ('syntax error', 'scripts = {', False, ['Lua error:']),
    ('runtime error', 'error("expected failure")', False, ['expected failure']),
    ('non-string error', 'error({})', False, ['non-string error']),
]
with tempfile.TemporaryDirectory(prefix='cases-', dir=ROOT / 'build') as temp:
    for name, code, ok, expected in cases:
        f = Path(temp) / 'config.lua'; f.write_text(code + '\n')
        check(name, [str(f.relative_to(ROOT))], ok, expected)
header = 'sku,name,on_hand,on_order,weekly_sales,pack,unit_cents'
valid = 'ONE,Example,0,0,1,1,100'
imports = [
    ('LF import', header+'\n'+valid+'\n', True, 'Imported 1'),
    ('CRLF import', header+'\r\n'+valid+'\r\n', True, 'Imported 1'),
    ('CR import without final newline', header+'\r'+valid, True, 'Imported 1'),
    ('bad header', 'wrong\n'+valid, False, 'invalid header'),
    ('empty inventory', header+'\n', False, 'inventory is empty'),
    ('duplicate SKU', header+'\n'+valid+'\n'+valid, False, 'duplicate SKU'),
    ('missing field', header+'\nONE,Example,0,0,1,1', False, 'invalid item fields'),
    ('extra field', header+'\n'+valid+',99', False, 'invalid item fields'),
    ('negative quantity', header+'\nONE,Example,-1,0,1,1,100', False, 'invalid item fields'),
    ('quantity overflow', header+'\nONE,Example,2147483648,0,1,1,100', False, 'invalid item fields'),
    ('zero pack', header+'\nONE,Example,0,0,1,0,100', False, 'invalid item fields'),
    ('zero price', header+'\nONE,Example,0,0,1,1,0', False, 'invalid item fields'),
    ('excessive price', header+'\nONE,Example,0,0,1,1,100001', False, 'invalid item fields'),
    ('fractional price', header+'\nONE,Example,0,0,1,1,1.5', False, 'invalid item fields'),
    ('quoted input', header+'\nONE,"Example",0,0,1,1,100', False, 'invalid item fields'),
    ('overlong name', header+'\nONE,'+'x'*32+',0,0,1,1,100', False, 'invalid item fields'),
    ('overlong line', header+'\n'+'x'*192, False, 'overlong line'),
    ('embedded binary byte', header+'\nONE,Ex\0ample,0,0,1,1,100', False, 'invalid or overlong line'),
    ('32 items', header+'\n'+'\n'.join('S%d,Item,0,0,1,1,100'%i for i in range(32)), True, 'Imported 32'),
    ('33 items', header+'\n'+'\n'.join('S%d,Item,0,0,1,1,100'%i for i in range(33)), False, 'more than 32 items'),
]
with tempfile.TemporaryDirectory(prefix='stock-', dir=ROOT / 'build') as temp:
    folder = Path(temp)
    data, script = folder / 'input.csv', folder / 'check.lua'
    for name, contents, ok, expected in imports:
        data.write_bytes(contents.encode('ascii'))
        script.write_text('local i=require("inventory"); local s=i.load('+json.dumps(str(data.relative_to(ROOT)))+')\n'
                          'local p=i.plan(s,function() return 0,"test" end,0); print("Imported "..p:summary().count)\n')
        check(name, ['--script', str(script.relative_to(ROOT))], ok, [expected])
    data.write_text(header+'\nONE,Expensive,0,0,1,1,100000\n')
    script.write_text('local i=require("inventory"); local s=i.load('+json.dumps(str(data.relative_to(ROOT)))+')\n'
                      'local p=i.plan(s,function() return 1000,"limit" end,100000000)\n'
                      'assert(p:summary().total_cents==100000000)\n'
                      'p=i.plan(s,function() return 10000,"defer" end,100000000)\n'
                      'assert(p:summary().total_cents==0 and p:row(1).decision=="DEFER")\n'
                      'print("Large costs checked")\n')
    check('32-bit cost and exact budget limit', ['--script', str(script.relative_to(ROOT))], True, ['Large costs checked'])
results.append({'name': 'inventory stays unchanged', 'passed': hashlib.sha256((ROOT / 'stock.csv').read_bytes()).hexdigest() == stock_before})
report = {'environment': 'GoldenGate --memcheck; not real hardware',
          'executable_sha256': hashlib.sha256(Path(exe).read_bytes()).hexdigest(),
          'demo_inputs': {n: hashlib.sha256((ROOT / n).read_bytes()).hexdigest() for n in ('config.lua', 'coltest.lua', 'stattest.lua', 'shopdemo.lua', 'policy.lua', 'stock.csv')},
          'test_sources': {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted((ROOT / 'tests').glob('*')) if p.is_file()},
          'total': len(results), 'failed': sum(not r['passed'] for r in results), 'checks': results}
(ROOT / 'build/TEST-REPORT.json').write_text(json.dumps(report, indent=2) + '\n')
print(str(report['total'] - report['failed']) + '/' + str(report['total']) + ' checks passed')
sys.exit(bool(report['failed']))
