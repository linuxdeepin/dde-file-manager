### dde-file-manager

Deepin File Manager is a file management tool independently developed by Deepin Technology, featured with searching, copying, trash, compression/decompression, file property and other file management functions.

### Dependencies

### Build dependencies

_The **master** branch is current development branch, build dependencies may changes without update README.md, refer to `./debian/control` for a working build depends list_
 
* pkg-config
* dh-systemd
* libxcb1-dev
* libxcb-ewmh-dev
* libxcb-util0-dev
* libx11-dev
* libgsettings-qt-dev
* libsecret-1-dev
* libpoppler-cpp-dev
* libpolkit-agent-1-dev
* libpolkit-qt5-1-dev
* libjemalloc-dev
* libmagic-dev
* libtag1-dev
* libdmr-dev
* x11proto-core-dev
* libdframeworkdbus-dev
* dde-dock-dev(>=4.0.5)
* deepin-gettext-tools
* libdtkcore-dev
* libudisks2-qt5-dev
* libdisomaster-dev
* libgio-qt-dev
* libqt5xdg-dev
* libmediainfo-dev
* libdde-file-manager-dev
* libssl-dev
* ffmpeg module(s):
  - libffmpegthumbnailer-dev
* Qt5(>= 5.6) with modules:
  - qtbase5-dev
  - qtbase5-private-dev
  - libqt5x11extras5-dev
  - qt5-qmake
  - libqt5svg5-dev
  - qttools5-dev-tools
  - qtmultimedia5-dev
  - qtdeclarative5-dev
  - libkf5codecs-dev
* Deepin-tool-kit(>=2.0) with modules:
  - libdtkwidget-dev
* deepin-anything with modules:
  - deepin-anything-dev
  - deepin-anything-server-dev

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
$ mkdir Build
$ cd Build
$ qmake ..
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
