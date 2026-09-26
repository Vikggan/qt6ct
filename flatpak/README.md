# Using this qt6ct fork with Flatpak applications

Host Qt plugins cannot be loaded directly by Flatpak applications. Build this
fork against each exact KDE runtime branch used by the applications instead.
Qt platform themes use private Qt APIs, so a plugin built for 6.11 must not be
used by an application on the 6.10 runtime.

## Build

Install the matching SDK and run the local build helper:

```sh
flatpak install flathub org.kde.Sdk//6.11
./flatpak/build-local.sh 6.11
```

Repeat this for another installed runtime branch, such as 6.10. Builds are
installed below `~/.local/share/qt6ct-flatpak/BRANCH` by default. Set
`QT6CT_FLATPAK_ROOT` to use another location.

## Enable one application

```sh
./flatpak/enable-app.sh org.kde.kwrite
```

The helper detects the application's KDE runtime branch and creates a per-app
user override. It exposes only the matching qt6ct build and the directory that
contains the selected qt6ct configuration. It also sets `QT6CT_CONFIG` so the
sandbox reads the host configuration instead of its app-specific config home.

To find applications that use the KDE runtime:

```sh
flatpak list --app --columns=application,runtime | grep org.kde.Platform
```

Inspect the resulting permissions with:

```sh
flatpak override --user --show org.kde.kwrite
```

Remove the qt6ct-specific override entries manually with `flatpak override`,
or use `flatpak override --user --reset APP_ID` only if discarding every custom
override for that application is acceptable.

Applications using a GNOME/Freedesktop runtime, or applications that bundle
their own Qt, are not ABI-compatible with these builds. They need qt6ct added
to their own Flatpak manifest and built with that application's Qt.

Styles, fonts, icon themes, and color schemes must also be visible inside the
sandbox. If a host style is unavailable there, Qt falls back to a style shipped
by the runtime. Older runtime branches can also reject a font description saved
by a newer Qt version; use a branch-specific copy of `qt6ct.conf` via the
`QT6CT_CONFIG` environment variable if that becomes a problem.
