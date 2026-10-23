# Arcified downstream development

This subtree is user-owned downstream work, independent of the upstream Helium contribution process.

The root `AGENTS.md` (also exposed through `CLAUDE.md`) permits agent work throughout this fork and defines the boundary with upstream imputnet repositories.

- Keep extension work in `arcified/` and prefer `patches/arcified/` for downstream native patches, with the required `patches/series` entries. Supporting build, CI, tooling and documentation changes may use the appropriate paths elsewhere in this fork.
- Do not submit AI-generated code, issues, or pull requests to imputnet's repositories.
- Keep Helium's existing quilt patches, patch ordering, copyright, and licensing intact.
- TypeScript extension work uses Bun and TypeScript 7. Native Chromium changes use the Chromium C++ build toolchain.
- Validate Linux, Windows and macOS behavior. Never claim that extension tests are native browser build tests.
- Explicit user approval is required before accessing and sending webpage content to any AI provider.
- Widevine integration is optional; do not bundle proprietary CDM binaries or secrets.
