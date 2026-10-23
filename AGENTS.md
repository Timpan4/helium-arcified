# Helium Arcified agent guidelines

This repository is `Timpan4/helium-arcified`, an independent, owner-authorized fork. Agents may inspect, suggest, generate, modify and test code and documentation throughout this repository, including native patches, build tooling, CI and repository instructions. There is no directory-based restriction on agent work within this fork.

## Upstream boundary

Upstream Helium's prohibition on AI-generated contributions applies to `imputnet` repositories. Keep agent work in this fork. Never push code, open pull requests or file issues in `imputnet/helium` or other `imputnet` repositories.

## Downstream development

- Preserve upstream copyrights, licenses and existing quilt patch order. Keep downstream native changes in a distinct patch group where practical.
- Follow `arcified/AGENTS.md` for the extension and Arcified-specific product requirements.
- Repository-wide write permission does not authorize unrelated changes, remote publication, merges, live-host changes or access to secrets. Follow the user's task scope and approvals for those actions.
