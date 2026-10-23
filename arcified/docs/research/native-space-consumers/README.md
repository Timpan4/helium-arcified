# Native Space consumer evidence

This directory supports [the consumer inventory](../../native-space-consumer-inventory.md). It contains source retrieval evidence and authored analysis, not an applied patch tree or a native feasibility proof.

- source-manifest.json lists the original successful pinned source, listing and tag requests with each response's hash, size and retrieval time. chromium/ preserves each decoded individual source file or shallow Gitiles tree JSON, beside its retrieval metadata.
- local-inputs.json pins the fork's working-tree inputs, selected downstream patches and v3 review.
- feature-members.md and feature-members.json enumerate the fetched header's 122 pointer members, direct construction sites, compile/runtime gates and cleanup. Proposed ownership is constructor-level analysis; internal dependencies remain open.
- search-ledger.json and patch-feature-header-search.json record the exact retained search surfaces, patterns, results and truncation state. They are not a Chromium-wide caller index.
- retrieval-notes.json records failed paths and retrieval limits. failures/ contains the later individual HTTP error records.
- verification.json separates local identity/document checks from unperformed native checks.

retrieve.mjs is the evidence capture helper. It uses read-only curl GETs against the fixed commit and writes only this evidence directory. Pass explicit source paths; a trailing slash requests a shallow tree, and @tag requests the pinned tag metadata. It performs no checkout, archive retrieval, source preparation, build or patch application. Example:

```sh
node arcified/docs/research/native-space-consumers/retrieve.mjs \
  'chrome/browser/ui/browser_window/public/browser_window_features.h'
```

Raw source retains upstream copyright/license headers. The pinned [Chromium BSD license](licenses/chromium-LICENSE) and its adjacent provenance receipt are supplemental license preservation, outside the original source-manifest count. Later lifecycle test inputs have their own evidence records. These captures are research inputs, not downstream production code. The manifest's retrieval count does not mean every captured file was fully reviewed. No actual native window, renderer, Profile storage, command dispatch or transfer behavior was exercised.
