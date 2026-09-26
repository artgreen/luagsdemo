# Modernization validation

Validated September 26, 2026 using GoldenGate with ORCA/C 2.2.11 and the
published Lua IIgs v0.3.0 full SDK. The SDK ZIP was downloaded from GitHub and
checked against its pinned SHA-256; no Lua source checkout was used to build
this host. The local ORCA/C compiler installation is shared with lua-iigs.

- `make doctor`: required tools and compiler found.
- `make package`: rebuilt both IIgs hosts; all **22/22** checks passed under
  `iix --memcheck`; both transfer containers passed exact member, byte, and
  ProDOS type/auxiliary-type verification.
- Extracted `LUAGSDEMO.SHK` into a separate directory and ran its executable
  with only the packaged scripts. The default demonstration completed.
- Modified the cached `parseconf.h` temporarily: SDK validation rejected it.
  Restored the original bytes and verified the full SDK again.
- All four files listed in `dist/SHA256SUMS` matched their recorded hashes.
- Git whitespace checks passed.

The regression suite covers valid and invalid configuration arrays, script
load/syntax/runtime errors, non-string errors, global-lookup errors, callbacks,
32-bit values, wrapping multiplication, collection bounds and lifetime,
status-name limits, repeated stack use, and closing/reopening the Lua state.
The suite runs the actual IIgs binaries, not a desktop Lua substitute.

Generated evidence is kept in `build/BUILD-MANIFEST.json`,
`build/TEST-REPORT.json`, and `build/installed-package.log`. The first two are
also included beside the transfer containers in `dist/`.

This is emulator validation. The new host still needs real IIgs acceptance:
extract the transfer package, launch `luademo` from an ORCA-compatible shell,
confirm the collection values and `42, New status`, and confirm it returns to
the shell without an error. System16 launching is outside the current package.
