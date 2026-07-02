---
name: release
description: 'Release a new Notepadqq version. Use when preparing a stable release, bumping the project version, updating AppStream release metadata, validating the release build, creating a git tag, or publishing GitHub release assets for Notepadqq.'
argument-hint: 'Target version and any release notes context, for example: 2.1.0'
user-invocable: true
---

# Release Notepadqq Version

Use this skill when you need to prepare and publish a new stable Notepadqq release.

This file is written to be provider-neutral so the same workflow can be used by agents that read project skills from `.github/skills/`, `.agents/skills/`, or `.claude/skills/`.

## What This Covers

- Bumping the source version in `CMakeLists.txt`
- Bumping the runtime/app version string in `src/ui/include/notepadqq.h`
- Adding the release entry in `support_files/com.notepadqq.Notepadqq.metainfo.xml`
- Updating release-facing packaging metadata in `snap/snapcraft.yaml`
- Updating the manpage header in `support_files/manpage/notepadqq.1`
- Running the release-oriented build and test checks used by CI
- Creating and pushing the release tag
- Preparing a GitHub release through the existing workflow

## Important Current Constraints

- The canonical project version is defined in `CMakeLists.txt` via `project(notepadqq VERSION ...)`.
- The application also exposes a user-facing version string via `POINTVERSION` in `src/ui/include/notepadqq.h`.
- AppStream release history is tracked in `support_files/com.notepadqq.Notepadqq.metainfo.xml`, not the old `support_files/notepadqq.appdata.xml` path.
- The manpage header in `support_files/manpage/notepadqq.1` includes a version string and release month/year that should be kept current for real releases.
- The Snap package declares its own version in `snap/snapcraft.yaml`.
- Git tags for stable releases should follow the `v<version>` pattern, for example `v2.1.0`.
- GitHub release titles should follow the `Notepadqq <version>` pattern, for example `Notepadqq 2.1.0`.
- The release workflow in `.github/workflows/release.yml` currently publishes macOS DMG assets only.
- Linux AppImages are produced by the nightly workflow in `.github/workflows/nightly.yml`, not by the stable release workflow.

## Inputs To Gather First

- Target version, for example `2.1.0`
- Release date in `YYYY-MM-DD` format for the metainfo entry
- Whether this is a prerelease or stable release
- Whether the user also wants to start the next development cycle immediately after the stable release
- Whether the user wants the agent to stop before pushing tags or opening the GitHub release

If any of these are missing, ask before changing files or running publishing steps.

## Procedure

1. Confirm the release intent.

- Verify the target version.
- Verify whether the release should be marked as development in the metainfo file.
- Verify whether a separate post-release development bump should be prepared.
- Verify whether remote side effects are allowed in this session.

2. Update the source version.

- Edit `CMakeLists.txt` and update `project(notepadqq VERSION ...)`.
- Edit `src/ui/include/notepadqq.h` and update `POINTVERSION` to the intended release string.
- Search for any other repo-owned version references that must stay aligned.
- Do not treat bundled third-party dependency versions as part of the app release.

Use the version surfaces deliberately:

- `CMakeLists.txt` is the canonical project version.
- `src/ui/include/notepadqq.h` is the user-visible app/runtime version string.
- Stable releases should not leave `+git` suffixes behind unless the user explicitly wants a development build label.

3. Update AppStream metadata.

- Add a new topmost `<release .../>` entry in `support_files/com.notepadqq.Notepadqq.metainfo.xml`.
- Use the confirmed release date.
- Add `type="development"` only for prereleases.

Do not use the historical `support_files/notepadqq.appdata.xml` path unless the repo reintroduces it.

4. Update ancillary release metadata.

- Update the version in `snap/snapcraft.yaml`.
- Update the date and version header in `support_files/manpage/notepadqq.1`.
- If the manpage copyright year is intentionally maintained separately, preserve repo conventions rather than guessing.

5. Validate locally with the same release-oriented path CI uses.

- Configure: `cmake --preset release`
- Build: `cmake --build --preset release --parallel`
- Test: `ctest --preset release --output-on-failure`

If the environment cannot run one of these, explain the limitation and run the narrowest available validation instead.

6. Review packaging implications.

- For macOS release assets, the current workflow packages DMGs through `build-tools/package-nightly-macos.sh` invoked by `.github/workflows/release.yml`.
- For Linux, note explicitly that stable release automation does not currently publish AppImages.
- If the user expects Linux release assets, call that out as a workflow gap rather than assuming they exist.

7. Prepare the release commit.

- Summarize exactly which version-bearing files changed.
- For a normal stable release, that usually includes `CMakeLists.txt`, `src/ui/include/notepadqq.h`, `support_files/com.notepadqq.Notepadqq.metainfo.xml`, `support_files/manpage/notepadqq.1`, and `snap/snapcraft.yaml`.
- If asked to commit, create a normal commit with a version-specific message.
- Do not create or amend commits unless the user explicitly asks.

8. Tag and publish.

- Create an annotated tag using the `v<version>` pattern, for example `v2.1.0`, unless the repo already documents a different convention.
- Push the branch and tag only with explicit user approval.
- Creating or publishing a GitHub release with that tag triggers `.github/workflows/release.yml` when the release is published.
- Name the GitHub release using the `Notepadqq <version>` pattern, for example `Notepadqq 2.1.0`, unless the user explicitly asks for a different title.
- Prefer drafting the GitHub release first when the user explicitly wants a draft release page.
- If the tag already exists, prefer using the workflow-dispatch path in `.github/workflows/release.yml` with the existing tag.
- If the Snap release should reach end users on the stable channel, remind the user to open the Snapcraft dashboard and promote the newly built revision to `stable` after the release is published.

9. Optionally start the next development cycle.

- Do this as a separate follow-up change after the stable release commit and tag, not by mutating the just-released version in place.
- Ask the user which next development version they want, for example `2.0.1+git` or `2.1.0-beta+git`.
- Apply the next development version to live development version surfaces such as `CMakeLists.txt`, `src/ui/include/notepadqq.h`, and usually `snap/snapcraft.yaml`.
- Do not add a future AppStream release entry or future manpage release date during this step.
- If asked to commit it, use a separate post-release commit so the stable release tag points at the exact shipped version.

## Working Rules

- Default to minimal edits in version-bearing files only.
- Validate immediately after the first substantive edit.
- Surface release workflow gaps instead of papering over them.
- Ask before any remote action: pushing, tagging, or publishing a release.
- Keep post-release development bumps separate from the stable release commit.

## Expected Output

When using this skill, the agent should finish by reporting:

- Updated version files
- Validation commands run and their outcome
- Whether the repo is ready for tagging
- Any release automation gaps, especially around Linux stable assets
- Any manual follow-up steps, including Snapcraft dashboard promotion to the `stable` channel when relevant