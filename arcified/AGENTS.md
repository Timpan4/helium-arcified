# Arcified downstream development

This subtree is user-owned downstream work, independent of the upstream Helium contribution process.

The root `AGENTS.md` (and `CLAUDE.md`) is inherited from upstream Helium and governs contributions to imputnet's repositories. In this fork, the owner permits agents to write code in `arcified/` and `patches/arcified/`, and to add the `patches/series` entries that reference those patches. Everything outside those paths stays under the upstream rules.

- Make changes only under arcified/ or patches/arcified/, plus their patches/series entries, when requested.
- Do not submit AI-generated code, issues, or pull requests to imputnet's repositories.
- Keep Helium's existing quilt patches, patch ordering, copyright, and licensing intact.
- TypeScript extension work uses Bun and TypeScript 7. Native Chromium changes use the Chromium C++ build toolchain.
- Validate Linux, Windows and macOS behavior. Never claim that extension tests are native browser build tests.
- Explicit user approval is required before accessing and sending webpage content to any AI provider.
- Widevine integration is optional; do not bundle proprietary CDM binaries or secrets.
