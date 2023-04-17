### dde-file-manager

Deepin File Manager is a file management tool independently developed by Deepin Technology, featured with searching, copying, trash, compression/decompression, viewing file property and other file management functions.

### Dependencies

### Build dependencies

_The **master** branch is current development branch, build dependencies may changes without update README.md, refer to `./debian/control` for a working build depends list_

- cmake,
- debhelper (>=9),
- pkg-config,
- dh-systemd,
- qtbase5-dev,
- qtbase5-private-dev,
- qtmultimedia5-dev,
- libffmpegthumbnailer-dev,
- libqt5svg5-dev,
- libpolkit-agent-1-dev, 
- libpolkit-qt5-1-dev,
- libdtkwidget-dev,
- libdtkgui-dev,
- libdtkcore-dev,
- libdtkcore5-bin,
- qttools5-dev-tools,
- dde-dock-dev(>=4.8.4.1),
- libdframeworkdbus-dev,
- libtag1-dev,
- libdmr-dev,
- libicu-dev,
- libdeepin-pdfium-dev,
- libqt5xdg-dev,
- libgio-qt-dev,
- libdfm-io-dev,
- libdfm-mount-dev,
- libdfm-burn-dev,
- libssl-dev,
- libgtest-dev,
- libgmock-dev,
- libgsettings-qt-dev,
- liblucene++-dev,
- libdocparser-dev,
- libboost-filesystem-dev,
- libsecret-1-dev,
- libkf5codecs-dev,
- libpoppler-cpp-dev,
- libcryptsetup-dev,
- deepin-desktop-base | deepin-desktop-server | deepin-desktop-device

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

_Package name may be different between distros, if dde-file-manager is available from your distro, check the packaging script delivered from your distro is a better idea._

Assume you are using [Deepin](https://distrowatch.com/table.php?distribution=deepin) or other debian-based distro which got dde-file-manager delivered:

``` shell
$ git clone https://github.com/linuxdeepin/dde-file-manager
$ cd dde-file-manager
$ sudo apt build-dep ./
```

2. Build:
```shell
$ cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
$ cmake --build build
```

3. Install:
```shell
$ sudo cmake --build build --target install
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

dde-file-manager is licensed under [GPL-3.0-or-later](LICENSE)
