# Lua IIgs embedding demo

A small C application that embeds [Lua 5.4.6 for the Apple IIgs](https://github.com/artgreen/lua-iigs).
It uses the published **v0.3.0 full SDK**, with the library, VM object, and public
headers verified as one matched set. No Lua checkout or vendored binaries are required.

The demo shows how to:

- Initialize the IIgs native stack guard and a Lua state.
- Register C modules and a standalone `mul()` function.
- Read a script list from a Lua configuration file into C-owned storage.
- Expose a collection as userdata with checked indices and safe finalization.
- Read and update a C-owned status structure through Lua functions.
- Run a Lua file or C string, retrieve a Lua global, and call a Lua function from C.
- Report errors, restore the Lua stack, and close the state.

## Build and run

On a development Mac, install GoldenGate, ORCA/C **2.2.x**, NuLib2, and Python **3.9+**.
The ORCA/C toolchain and the Lua release SDK are separate dependencies.

```sh
cp local.mk.example local.mk
# Edit GOLDEN_GATE in local.mk to point to your ORCA/C SDK.
make doctor
make sdk
make run
make test
```

`make sdk` downloads the versioned SDK ZIP and verifies its pinned SHA-256 before
extracting `LUALIB.SHK` to `.deps/full/`. Subsequent builds work offline and
recheck all library/header hashes. To use a downloaded ZIP offline, place it at
`.deps/lua-iigs-0.3.0-sdk.zip` first. A modified cache fails verification instead
of silently mixing builds. See `tools/demo.py` for the pinned URL and hash.

`make` compiles all six C translation units, links both `lvm.a` and `lua.lib`,
and writes `build/luademo`. Each build is fresh; compiler/linker errors fail the
command. `make run` launches the demo under GoldenGate with memory checking.
`make clean` removes `build/`, retaining the verified SDK cache and transfer packages.
Machine-specific settings belong in ignored `local.mk`.

The normal run ends with:

```text
C -> Lua -> C: 21 -> 42
C sees status: 42, New status
Demo completed
```

## Scripts and bindings

`config.lua` defines `scripts`, an array of at most eight nonempty paths of up to
63 bytes each. Paths resolve from the launch directory. `coltest.lua` demonstrates
the collection; `stattest.lua` updates the host's status. For direct GoldenGate
commands, export `GOLDEN_GATE` in your shell as well as setting it in `local.mk`:

```sh
export GOLDEN_GATE=/absolute/path/to/orca-sdk-2.2.1
iix --memcheck build/luademo
iix --memcheck build/luademo config.lua
iix --memcheck build/luademo --script coltest.lua
```

```lua
local collection = require("collection")
local values <close> = collection.new(10)
values:set(1, mul(40000, 2))
print(values:get(1)) -- 80000, preserving 32-bit Lua integers
```

Collections have **one-based** indices, sizes from 1 to 4096, and `size`, `get`,
`set`, and `close` methods. Explicit close, Lua 5.4 scope exit, and garbage
collection share an idempotent finalizer. Access after close raises a Lua error.
The host owns `status`; Lua may change its integer ticks and its name (at most
19 bytes, no embedded NUL). `mul` follows Lua integer wrapping arithmetic.

Only public Lua headers are used. The stack anchor lives in `main` through
`lua_close`, as required by the SDK. The `lg_` wrapper owns one state and uses
zero for success. It is an educational, single-threaded interface; scripts are
trusted application code and can use the standard libraries. It is not a sandbox.

## Test and transfer to an IIgs

`make test` builds both the application and a C host test, then exercises the
actual IIgs binaries through GoldenGate `--memcheck`. Tests cover the default
flow, configuration errors, Lua error recovery, callbacks, integer widths,
bounds, explicit/automatic cleanup, and status string limits. Results and exact
executable hashes are written to `build/TEST-REPORT.json` and `BUILD-MANIFEST.json`.

With AppleCommander **acx** and CiderPress II **cp2** installed:

```sh
make package
```

This rebuilds and tests before producing `dist/LUAGSDEMO.SHK` and
`dist/luagsdemo.po`, plus manifests and checksums. Every container member is
read back and compared byte-for-byte, including ProDOS type/aux metadata.
Use GS ShrinkIt or copy from the transfer image preserving file types. Run
`luademo` from an ORCA-compatible shell in the extracted directory.
See [IIgs installation](docs/INSTALL.txt) and [validation results](docs/VALIDATION.md).

The new host has been tested under GoldenGate. **Real IIgs acceptance is still
required.** The old project's System16 launch claim has not been revalidated;
the current package supplies an ORCA shell EXE only.

## Changes from the original demo

The obsolete bundled headers, library, VM object, and executable are removed.
The former zero-based collection API is replaced by the methods shown above.
The uninitialized configuration pointer, unchecked allocations, double-free,
unbounded status copy, ignored script errors, and 16-bit C integer truncation
are fixed. The old Lua-internal string-table dumper is removed. The standalone
`poker.lua` remains as historical example source; it is not run or packaged by
this embedding demo. Current game examples live in the Lua IIgs distribution.

## Acknowledgments

Part of giving back to the Apple community by bringing useful tools to the IIgs.
Powered by [ORCA/C](https://github.com/byteworksinc/ORCA-C),
[GoldenGate](https://juiced.gs/store/golden-gate/), and [Lua](https://www.lua.org/).
The original C-function example drew on
[Lucas Klassmann's embedding tutorial](https://lucasklassmann.com/blog/2019-02-02-embedding-lua-in-c/).
The downloaded SDK retains Lua's license; transfer packages include it.
