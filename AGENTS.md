# Agent Directives for VFDDisplay

Scope: This file applies to the entire VFDDisplay repository (this folder and all subfolders).

Non‑Negotiable Safety Rules
- NEVER delete, rename, or remove the `.git/` directory.
- NEVER delete, rename, or remove the top‑level `src/` directory.
- NEVER delete, rename, or remove the top‑level `examples/` directory.
- Do not perform destructive operations (e.g., `rm -rf`, `git reset --hard`) without explicit maintainer approval.

No‑Touch Code Markers
- Methods and regions in the device HAL marked with a "NO_TOUCH" comment are considered hardware‑primitive implementations.
- Do NOT edit, refactor, or remove code within methods or regions annotated with NO_TOUCH.
- When extending functionality, build on higher‑level layers or add new methods; do not alter NO_TOUCH blocks.

Locked Interfaces and Existing HALs
- The `IVFDHAL` interface is considered LOCKED. Do not modify its methods or signatures.
- Existing device HALs (e.g., `VFD20S401HAL`) and `BufferedVFD` are LOCKED. Do not change their behavior or public APIs.
- When adding a new device HAL, all changes must remain within the new HAL implementation (and its documentation/capabilities). Adjust NO_TOUCH primitives only within that new HAL as required by its datasheet.

Commit Policy
- Commit all changes you make as small, focused commits with clear messages.
- Keep the `CHANGELOG.md` up to date in the same commit(s) describing what changed and why.
- Prefer surgical edits over sweeping refactors; preserve existing structure and docs.

Build & Project Conventions
- The library must compile under PlatformIO and Arduino CLI.
- Include `.ino` wrappers in each example folder so Arduino IDE can build, and keep per‑example PlatformIO configs where helpful.
- Exclude non‑C++ utilities from C++ builds (e.g., `utility/` Python) via PlatformIO `srcFilter`. Do NOT move or delete those folders without explicit approval.

Collaboration Etiquette
- Before making changes, run `git status` to understand the working tree state.
- When in doubt, ask for confirmation instead of guessing; prefer minimal, reversible changes.

Standard Procedure
- After completing changes, ensure `CHANGELOG.md` is updated to describe what changed and why.
- Commit changes as small, focused commits with clear messages, following the Commit Policy above.
- When adding developer tooling or CI/test scaffolding, also add brief usage notes in `docs/`.
