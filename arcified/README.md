# Helium Arcified

Downstream work on an Arc-style browser interface and an opt-in Dia-style assistant, based on the [Helium](https://github.com/imputnet/helium) Chromium patchset.

This is an **independent fork**, not an upstream Helium release. Target platforms are **Windows and macOS**.

## Repository layout

- `patches/`: Helium's existing quilt patches. Keep these in upstream order.
- `arcified/extension/`: cross-platform Manifest V3 prototype built with Bun and TypeScript 7.
- `arcified/docs/`: architecture, milestones and native build notes.
- `.github/workflows/arcified-extension.yml`: CI for the extension, not the full browser.

The extension is a **prototyping tool**, not a complete Arc replacement. Workspace snapshots open as separate windows; they are not native live Spaces. Chrome's extension side panel is not Helium's built-in sidebar.

## Try the prototype

Requires Bun 1.4.3 or newer.

1. Run `cd arcified/extension`
2. Run `bun install` and `bun run check`
3. In Helium open `helium://extensions`, enable Developer mode and choose **Load unpacked**
4. Select `arcified/extension/dist`
5. Click the **Arcified Lab** toolbar action

Local AI is optional. In LM Studio start its server on 127.0.0.1:1234 and load a model, then explicitly capture page context and ask the model. The extension does not automatically send pages to a cloud provider.

## Fork development approach

- Keep `main` aligned with `imputnet/helium`.
- Develop features on short-lived branches and use PRs in **Timpan4/helium-arcified**, not upstream.
- Keep browser-facing native work in quilt patches; prefer a separate `patches/arcified/` grouping once the first patch exists.
- Test feature parity on Windows and macOS before considering native releases.
- Never redistribute or copy proprietary Widevine CDMs. Streaming playback is optional best-effort.

See [roadmap](docs/roadmap.md) and [native integration](docs/native-integration.md).

## Licensing and upstream policy

The fork retains Helium's GPL-3.0 and third-party licenses. All new Arcified code in this directory is GPL-3.0-only. Follow Helium's upstream contributor rules and do not submit AI-generated contributions to `imputnet` repositories.
