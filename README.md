qt6ct - Qt6 Configuration Tool

This program allows users to configure Qt6 settings (theme, font, icons, etc.)
under DE/WM without Qt integration.

Official home page: https://www.opencode.net/trialuser/qt6ct

Requirements:

- GNU Linux or FreeBSD
- qtbase >= 6.2.0
- qtsvg >= 6.2.0 (Runtime dependency for the SVG icon support)
- qttools >= 6.2.0 (For build only)


Installation:

- Arch package
  https://archlinux.org/packages/?q=qt6ct

- Building from source code using cmake (recommended)
```
  cmake -DCMAKE_INSTALL_PREFIX=<your installation path>
  make
  sudo make install
```
**Additional information:** to override default platform plugin path use **PLUGINDIR** cmake variable.

- Building from source code using qmake
```
  qmake6 PREFIX=<your installation path>
  make
  sudo make install
```

Add line 'export QT_QPA_PLATFORMTHEME=qt6ct' to ~/.profile and re-login.
Alternatively, create the file /etc/X11/Xsession.d/100-qt6ct with
the following line:
```
export QT_QPA_PLATFORMTHEME=qt6ct
```
(or qt5ct for compatibility with Qt5 Configuration Tool)

Now restart X11 server to take the changes effect.

Extra build options (for advanced users only):
```
qmake DISABLE_WIDGETS=1 - compiles platform plugin without QtWidgets (useful for QML applications only)
qmake PLUGINDIR=<custom path> - changes the default installation path of the plugins (libqt6ct.so and libqt6ct-style.so)
```
Wayland:

If you are running Qt application under Wayland, you should export the following variables:
```
export QT_QPA_PLATFORM=wayland
export QT_QPA_PLATFORMTHEME=qt6ct
```
Debug Messages:

By default, debug messages from platform plugin are disabled. To enable them, you should
export the following environment variable:
```
export QT_LOGGING_RULES="qt6ct.debug=true"
```
Another way is to change qtlogging.ini file. See Qt documentation for details:
https://doc.qt.io/qt-6/qloggingcategory.html (paragraph "Configuring Categories")

Files and Directories:
```
qt6ct - Qt6 configuration tool
libqt6ct.so - qt6ct platform plugin
libqt6ct-style.so - qt6ct proxy style plugin
libqt6ct-common.so - qt6ct shared library
~/.config/qt6ct/qt6ct.conf - configuration file
~/.config/qt6ct/qss, ~/.local/share/qt6ct/qss, /usr/share/qt6ct/qss, /usr/local/share/qt6ct/qss - style sheets
~/.config/qt6ct/colors,~/.local/share/qt6ct/colors, /usr/share/qt6ct/colors,
/usr/local/share/qt6ct/colors - color schemes
/etc/xdg/qt6ct/qt6ct.conf - default configuration file
```

Translation:

Use Transifex service: https://explore.transifex.com/qt-configuration-tool/qt6ct/
