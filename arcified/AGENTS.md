# Arcified downstream development

This subtree is user-owned downstream work, independent of the upstream Helium contribution process.

- Make changes only under arcified/ or downstream-specific patch locations when requested.
- Do not submit AI-generated code, issues, or pull requests to imputnet's repositories.
- Keep Helium's existing quilt patches, patch ordering, copyright, and licensing intact.
- TypeScript extension work uses Bun and TypeScript 7. Native Chromium changes use the Chromium C++ build toolchain.
- Validate Windows and macOS behavior. Never claim that extension tests are native browser build tests.
- Explicit user approval is required before accessing and sending webpage content to any AI provider.
- Widevine integration is optional; do not bundle proprietary CDM binaries or secrets.
