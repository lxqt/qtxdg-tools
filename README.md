# qtxdg-tools

## Overview

User tools for [libqtxdg](https://github.com/lxqt/libqtxdg).

`qtxdg-tools` contains a CLI MIME tool, `qtxdg-mat`, for handling file associations and opening files with their default applications.

It is maintained by the LXQt project and needed by LXQt Session, in order to be used by `xdg-utils`. Yet it can be used independently from LXQt, too.

## Installation

### Sources

At runtime qtbase is needed. Additional build dependencies are CMake, [libqtxdg](https://github.com/lxqt/libqtxdg) and, optionally, Git to pull latest VCS checkouts.

Code configuration is handled by CMake. CMake variable `CMAKE_INSTALL_PREFIX`
will normally have to be set to `/usr`.

To build run `make`, to install `make install`, which accepts variable `DESTDIR`
as usual.
