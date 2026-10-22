# Arcified roadmap

## M0: Baseline (this PR)

- [x] Use a real Helium fork for native patch experiments.
- [x] Add a Bun / TypeScript 7 MV3 extension sandbox without changing the inherited Chromium patch series.
- [x] Add Windows, macOS and Linux extension checks.
- [ ] Install the unpacked extension in Helium on Linux, Windows and macOS and capture UI observations.

## M1: Arc tab lifecycle

- [ ] Define live Spaces and per-space pinned tabs. A window snapshot is not a live Space.
- [ ] Specify behavior for closing pinned tabs, audio, downloads and unsaved forms.
- [ ] Add a reusable state machine and unit tests before native patches.
- [ ] Integrate the address bar and tab navigation with Helium's native sidebar.

## M2: Native Arc interface

- [ ] Implement the space switcher as a Chromium/Helium quilt patch.
- [ ] Add Peek, pinned tab reset, temporary tab cleanup and split view integration.
- [ ] Build and test the fork separately on Linux, Windows and macOS.

## M3: Dia-style intelligence

- [ ] Opt-in @tab and @space context selection, with source citations.
- [ ] Provider adapters for local models and separately authenticated API providers.
- [ ] Scoped permissions, safe tool execution, and streaming responses.
- [ ] Encrypted cross-device user-data sync (with an explicit threat model).

## M4: Release

- [ ] Branding and native package identities for all three platforms.
- [ ] Distributable builds with updates and code signing.
- [ ] Optional Widevine playback compatibility testing; never ship unlicensed CDMs.
