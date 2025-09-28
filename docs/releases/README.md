# Releases

This repository uses annotated tags (e.g., `v1.0.1`). A GitHub Actions workflow can publish a GitHub Release when a tag is pushed or via manual trigger.

How to publish a release
- Automatic on tag push: push an annotated tag like `v1.2.3` and the `Release` workflow will create a GitHub Release.
- Manual for an existing tag: run the `Release` workflow with `workflow_dispatch` and provide the tag (e.g., `v1.0.1`).

Optional release notes file
- If a file exists at `docs/releases/<tag>.md` (e.g., `docs/releases/v1.0.1.md`), the workflow will use it as the release body.
- Otherwise, it will generate release notes automatically from commits and PRs.

Creating a release notes file
- Copy the top section from `CHANGELOG.md` and save it to `docs/releases/<tag>.md`.
- Keep the notes concise and focused on user-visible changes.

