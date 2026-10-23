# Native Helium integration

Helium's main repository contains a patch series for Chromium, not an entire checked-in Chromium tree.

The native feature implementation will use C++/Chromium and quilt patches. The TypeScript extension in ../extension is only a test harness for state and UX.

The [downstream Linux preparation guide](native-linux.md) documents the local source/patch identity preflight and an explicit preparation route using pinned Linux platform tooling and this fork's inputs. Preparation is a plan by default and stops after GN generation when executed on an authorized build host. It does not compile or launch a browser. Issue #16's build and native interaction acceptance remains open.

## Constraints

- Maintain a downstream patch group, eventually patches/arcified, referenced in patches/series in the required order.
- First create and test changes within the platform's Chromium source tree, then refresh quilt patches.
- Preserve upstream copyrights and licenses. Rebase the smallest patches possible whenever Chromium advances.
- Build with the separate Linux, Windows and macOS packaging/dev environments; Bun alone cannot compile a browser.
- No native browser binaries or CI results are implied by extension typecheck.

References:
- https://github.com/imputnet/helium-linux
- https://github.com/imputnet/helium-macos/blob/main/docs/building.md
- https://github.com/imputnet/helium-windows
- https://github.com/imputnet/helium
