# Application validation

Validated September 26, 2026 using GoldenGate with ORCA/C 2.2.11 and the
published Lua IIgs v0.3.0 full SDK. The SDK ZIP is pinned by SHA-256; builds use
its matching library, VM object, and public headers. The local compiler
installation is shared with lua-iigs, but no Lua source checkout is needed.

## Executable and package checks

- `make check`: **5/5** offline SDK tooling checks passed. GitHub CI runs
  these checks without GoldenGate or a compiler SDK.

- `make package` rebuilt both IIgs hosts. **48/48** automated checks passed
  under `iix --memcheck`. The suite now runs in a disposable directory;
  existing reports in the checkout were unchanged.
- Every member of both transfer containers matched its source bytes and
  expected ProDOS type/auxiliary type, with exactly the expected member set.
- Extracted `LUAGSDEMO.SHK` into a separate directory and ran the default
  application using only packaged files, including native CR inventory data.
  Standard and lean reports totaled 19200 and 9600 cents, respectively.
- Edited only the extracted `POLICY.LUA`, raising the standard budget from
  20000 to 30000 cents. The same executable then accepted tea and produced a
  26000-cent plan. Its binary SHA-256 stayed unchanged.
- Git whitespace checks passed.

The application tests cover malformed CSV, duplicate SKUs, line endings,
record/quantity limits, policy failures after earlier successful callbacks,
invalid quantities and reasons, protected C-owned values, exact budget
boundaries, large 32-bit costs, export quoting, and unchanged input inventory.
The earlier binding and C-host regression coverage remains in the suite:
configuration errors, Lua error recovery, callbacks, integer widths, collection
bounds and cleanup, status-name limits, and repeated state/stack use.

The earlier SDK cache check also verified that a modified `parseconf.h` is
rejected; the original bytes were restored and revalidated afterward.

## Evidence

Generated records are in `build/BUILD-MANIFEST.json` and
`build/TEST-REPORT.json`, also copied into `dist/`. These include hashes for
both C inputs and runtime Lua/data inputs. `dist/SHA256SUMS` covers the two
transfer containers and both manifests. The installed-package run and policy
edit for the unchanged application executable are recorded in `build/shop-installed.log` and `build/shop-installed.json`.

## Real IIgs acceptance

The maintainer reported “hardware acceptance testing complete and good” on
September 26, 2026 and approved publication. [ACCEPTANCE.json](ACCEPTANCE.json)
records this report and the accepted executable and Lua/data hashes. Machine
details and a detailed transcript were not supplied. GoldenGate results remain
a separate source of validation.

For future builds, repeat the acceptance procedure: extract
the package into a writable directory and run `luademo` in an ORCA-compatible
shell. Confirm the $192/$96 plans, inspect `ORDERS.CSV` and `LEAN.CSV`, and
confirm the smaller examples complete and return to the shell. Edit the
standard budget in `POLICY.LUA` to 30000 cents and run again: tea should be
ordered and the standard total should become $260.00. Restore the sample
policy when finished. System16 launching is outside the current package.
