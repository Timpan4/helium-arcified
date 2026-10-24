# Native consumer inventory response

Corrected all three findings from [the independent review](native-space-consumer-review.md). Fresh closure review remains pending. This changes the source catalog, not production behavior or native acceptance.

1. The JSON and Markdown feature catalogs now record the conditional Views provider cleanup through `browser_elements_->AsA<BrowserElementsViews>()` at implementation lines 1253 to 1254. It remains before browser animation and actor-border teardown. Removed the line-1153 null assignment from `pinned_toolbar_actions_` initialization; its actual acquisition at line 1041 and teardown at line 1153 remain. The existing lazy find-bar construction at line 1281 is unchanged.
2. E16 now identifies `ShowCertificateViewer` and distinguishes the inspected page when docked from the DevTools frontend when undocked. Its proposed native-parent extraction and checks preserve that distinction.
3. E17 separately cites insertion at lines 92 to 101 and replacement at lines 142 to 147. Its constructor citation now stops before insertion handling.

Compared each corrected interpretation with the retained pinned implementation. The 122 declaration entries and 75 captured source files are unchanged. Retained source, manifests, retrieval receipts and the original review are preserved. Whitespace and catalog consistency checks are recorded with the closure review.

The first bounded destruction extraction must preserve the entire existing feature pre-window hook and its ordering. The corrected catalog does not prove every enabled feature's internal or reverse-caller closure. Same-window multi-Profile feasibility, exact live-page transfer and Linux/Windows/macOS native acceptance remain OPEN. Full preparation and browser builds remain deferred.
