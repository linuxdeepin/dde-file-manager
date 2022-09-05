### dde-file-manager

Deepin File Manager is a file management tool independently developed by Deepin Technology, featured with searching, copying, trash, compression/decompression, file property and other file management functions.

### Dependencies

### Build dependencies

_The **master** branch is current development branch, build dependencies may changes without update README.md, refer to `./debian/control` for a working build depends list_

* pkg-config
* qt5-qmake
* qtbase5-dev
* qtbase5-private-dev
* libqt5x11extras5-dev
* libjemalloc-dev
* libqt5svg5-dev
* libmagic-dev
* libsecret-1-dev
* libpoppler-cpp-dev
* libpolkit-agent-1-dev
* libpolkit-qt5-1-dev
* qtmultimedia5-dev
* libffmpegthumbnailer-dev
* libtag1-dev
* libdtkwidget-dev
* libdtkgui-dev
* libxcb1-dev
* x11proto-core-dev
* libx11-dev
* qttools5-dev-tools
* libglib2.0-dev
* dde-dock-dev(>=4.8.4.1)
* libxcb-ewmh-dev
* libxcb-shape0-dev
* libdframeworkdbus-dev
* libxcb-util0-dev
* qtdeclarative5-dev
* deepin-gettext-tools
* libkf5codecs-dev
* libdtkcore-dev
* libdtkcore5-bin
* libdmr-dev
* deepin-anything-dev[i386 amd64]
* deepin-anything-server-dev[i386 amd64]
* libudisks2-qt5-dev(>=5.0.6)
* libxrender-dev
* libmtdev-dev
* libfontconfig1-dev
* libfreetype6-dev
* libegl1-mesa-dev
* libglibmm-2.4-dev
* libgio-qt-dev(>=0.0.11)
* libdisomaster-dev(>=5.0.7)
* libssl-dev
* libmediainfo-dev
* libqt5xdg-dev
* libicu-dev
* liblucene++-dev
* libgsf-1-dev
* libboost-filesystem-dev
* libdocparser-dev
* deepin-desktop-base | deepin-desktop-server | deepin-desktop-device
* libpcre3-dev

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

_Package name may be different between distros, if dde-file-manager is available from your distro, check the packaging script delivered from your distro is a better idea._

Assume you are using [Deepin](https://distrowatch.com/table.php?distribution=deepin) or other debian-based distro which got dde-file-manager delivered:

``` shell
$ apt build-dep dde-file-manager
```

2. Build:
```
$ cd dde-file-manager
$ mkdir build
$ cd build
$ qmake ../filemanager.pro
$ make
```

3. Install:
```
$ sudo make install
```

The executable binary file could be found at `/usr/bin/dde-file-manager`

## Usage

Execute `dde-file-manager`

## Documentations

 - [Development Documentation](https://linuxdeepin.github.io/dde-file-manager/)
 - [User Documentation](https://wiki.deepin.org/wiki/Deepin_File_Manager) | [用户文档](https://wiki.deepin.org/index.php?title=%E6%B7%B1%E5%BA%A6%E6%96%87%E4%BB%B6%E7%AE%A1%E7%90%86%E5%99%A8)

## Getting help

 - [Official Forum](https://bbs.deepin.org/)
 - [Developer Center](https://github.com/linuxdeepin/developer-center)
 - [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
 - [IRC Channel](https://webchat.freenode.net/?channels=deepin)
 - [Wiki](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

 - [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) (English)
 - [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)
 - [Translate for your language on Transifex](https://www.transifex.com/linuxdeepin/deepin-file-manager/)

## License

dde-file-manager is licensed under [GPLv3](LICENSE)
