# Publishing the demo

Publish to the existing `artgreen/luagsdemo` repository. Preserve its history;
this modernization uses a feature branch and a normal pull request into
`master`. The Lua SDK version and the demo's own release version are independent.

## Prepare

1. Start from a clean checkout with `local.mk` pointing to ORCA/C 2.2.x.
2. Run `make doctor`, `make check`, and `make package`.
3. Confirm both manifests identify the current C and Lua/data inputs and the
   same executable. Check every entry in `dist/SHA256SUMS`.
4. Extract the ShrinkIt archive into a separate writable directory. Run the
   complete job, inspect the $192/$96 reports, then raise the standard policy's
   budget to 30000 cents and confirm the $260 plan using the same binary.
5. Run the hardware procedure in [VALIDATION.md](VALIDATION.md). Record actual
   results without treating emulator success as hardware acceptance. If hardware
   results are unavailable, identify the release as a prerelease for testing.
6. Update the changelog, validation record, and release notes. Include report
   replacement behavior and the ORCA shell EXE scope.

Generated files live in `build/` and `dist/`; the SDK cache, local settings, and
compiler SDK remain ignored. Do not add them to Git. Old tests/logs may be
removed after preserving evidence for the candidate under review.

## Publish after review

State the local branch, `origin` push remote, `artgreen/luagsdemo` PR repository,
`master` base, and feature-branch head before pushing. Push the feature branch,
create a PR, and verify the SDK tooling workflow. That workflow does not run the
IIgs binaries; the full GoldenGate report accompanies the release candidate.
Merge after review. No force push or default-branch guard exception is needed.

Choose a demo version/tag at the reviewed merge commit and create a draft
GitHub release. Attach:

- `LUAGSDEMO.SHK` and `luagsdemo.po`;
- `BUILD-MANIFEST.json` and `TEST-REPORT.json`;
- `SHA256SUMS`.

Verify uploaded sizes and SHA-256 digests before making the release public.
GitHub provides source archives for the tagged commit. Keep source changes
since candidate validation limited to documentation or revalidate affected code
and regenerate the artifacts. Record the final source commit in the release
notes; the manifests identify the actual build and runtime inputs.
