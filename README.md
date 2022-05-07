## Overview

User tools for [libqtxdg](https://github.com/lxqt/libqtxdg/).

`qtxdg-tools` contains a CLI mime apps toolset, `qtxdg-mat`. LXQt uses the `$desktop-mimeapps.list` file to store the file  associations and defaults.

It is maintained by the LXQt project and nearly all LXQt components are depending on it. Yet it can be used independently from this desktop environment, too.


## Installation

### Sources

At runtime qtbase is needed. Additional build dependencies are CMake, [libqtxdg](https://github.com/lxqt/libqtxdg/) and optionally Git to pull latest VCS checkouts.

Code configuration is handled by CMake. CMake variable `CMAKE_INSTALL_PREFIX`
will normally have to be set to `/usr`.

To build run `make`, to install `make install` which accepts variable `DESTDIR`
as usual.





