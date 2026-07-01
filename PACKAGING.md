# Packaging Notes

## Canonical install layout

- executable: `/usr/bin/notepadqq`
- application data: `/usr/share/notepadqq`
- desktop file: `/usr/share/applications/notepadqq.desktop`
- metainfo: `/usr/share/metainfo/com.notepadqq.Notepadqq.metainfo.xml`

Upstream CMake installs the real executable directly as `notepadqq` and does not install a generic launcher wrapper.

## Packager responsibilities

Distribution-specific wrappers should only be added by packagers when they are required for that package format or runtime environment.

Historically, some packages used wrappers to apply desktop-environment-specific Qt workarounds such as:

- `QT_QPA_PLATFORMTHEME=""`
- `XDG_CURRENT_DESKTOP="GNOME"`

These are not treated as upstream runtime requirements. If a downstream package still needs one of these workarounds, keep it in that package's launcher, desktop entry, or equivalent runtime configuration.

## Snap

The Snap package keeps its own launcher and Qt runtime configuration under `snap/local/`. That logic is package-specific and intentionally remains outside the upstream CMake install rules.
