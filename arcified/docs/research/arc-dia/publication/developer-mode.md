<!-- arcified-backlog:developer-mode -->

Part of [Track navigation and multitasking](https://github.com/Timpan4/helium-arcified/issues/6) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Expose developer tools, view-source and site-specific developer mode without breaking compact browsing.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Open/dock/undock DevTools for the correct tab or split pane and retain expected keyboard shortcuts.
- [ ] Developer mode can show full URL and relevant diagnostic controls without leaking between Profiles.
- [ ] Closing or hiding DevTools does not leave background update work or detach the wrong page.
- [ ] Record explicit dispositions for automatic localhost activation, JSON readability and portrait capture. Reuse native facilities where present and do not silently install extensions.
- [ ] For opted-in local servers, distinguish ports/full origins, target diagnostics to the right tab, redact secrets in copied evidence and stop hidden developer updates when tools close.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Boot a downstream native Arcified build on Linux](https://github.com/Timpan4/helium-arcified/issues/16) (`native-linux`)
- [Boot a downstream native Arcified build on Windows](https://github.com/Timpan4/helium-arcified/issues/17) (`native-windows`)
- [Boot a downstream native Arcified build on macOS](https://github.com/Timpan4/helium-arcified/issues/18) (`native-macos`)

## Evidence and design intent

YAB official manual/product documentation, reviewed 2026-10-23. Its inconsistent version labels and vendor size/performance claims do not establish runtime results.

YAB sources describe macOS/WebKit. Arc/Dia documentation contributes only where explicitly cited. Linux and Windows are independent downstream targets.

- [Arc: Developer Mode Instant Dev Tools](https://resources.arc.net/hc/en-us/articles/20468488031511-Developer-Mode-Instant-Dev-Tools)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Dia: changelog](https://www.diabrowser.com/changelog)
- [YAB: manual / developer mode](https://yetanotherbrowser.com/manual/developer-mode)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
