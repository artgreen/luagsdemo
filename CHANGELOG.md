# Changelog

## 0.1.0 — 2026-09-26

- Replace bundled Lua headers and binaries with a verified Lua IIgs v0.3.0 SDK.
- Add a shop restocking application: C owns inventory, validation, budgets,
  and CSV reports; Lua supplies two editable policies.
- Keep focused collection, shared-status, callback, and configuration examples.
- Fix stack initialization, pointer ownership, collection lifetime and bounds,
  string limits, error handling, and 32-bit integer behavior.
- Build and verify ShrinkIt and ProDOS transfer containers with test evidence.
- Run regression tests in an isolated directory; add offline SDK tooling CI.
- Remove the obsolete executable, internal headers, poker script, and screenshot.

GoldenGate validation is recorded in `docs/VALIDATION.md`. The maintainer
reported successful hardware acceptance; `docs/ACCEPTANCE.json` records the
accepted executable and Lua/data inputs.
