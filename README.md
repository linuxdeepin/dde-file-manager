### dde-file-manager

Deepin File Manager is a file management tool independently developed by Deepin Technology, featuring functions such as searching, copying, trash management, compression/decompression and viewing file properties.

### Dependencies

### Build dependencies

_The **master** branch is current development branch, build dependencies may change without updating README.md. Please refer to `./debian/control` for an accurate list of build dependencies_

- cmake,
- doxygen,
- debhelper (>=9),
- pkg-config,
- libffmpegthumbnailer-dev,
- libpolkit-agent-1-dev,
- dde-tray-loader-dev,
- libtag1-dev,
- libdmr-dev,
- libicu-dev,
- libxcb-ewmh-dev,
- libdeepin-pdfium-dev,
- libssl-dev,
- libgtest-dev,
- libgmock-dev,
- liblucene++-dev,
- libdocparser-dev,
- libboost-filesystem-dev,
- libsecret-1-dev,
- libpoppler-cpp-dev,
- libcryptsetup-dev,
- libpcre2-dev,
- libdde-shell-dev (>= 0.0.10),
- deepin-desktop-base | deepin-desktop-server | deepin-desktop-device,
- qt6-base-dev,
- qt6-base-private-dev,
- qt6-svg-dev,
- qt6-multimedia-dev,
- qt6-tools-dev,
- qt6-tools-dev-tools,
- qt6-declarative-dev,
- qt6-5compat-dev,
- libdtk6widget-dev,
- libdtk6gui-dev,
- libdtk6core-dev,
- libdtk6core-bin,
- libdtk6declarative-dev,
- libdfm6-io-dev,
- libdfm6-mount-dev,
- libdfm6-burn-dev,
- libdfm6-search-dev,
- libpolkit-qt6-1-dev,
- libxcb-xfixes0-dev (>= 1.10~),
- libopenjp2-7-dev,
- liblcms2-dev,
- libdeepin-service-framework-dev,
- libheif-dev

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

_The package name may vary between different distros. If dde-file-manager is available in your distro, it is a better idea to check the packaging script provided by your distro._

Assume you are using [Deepin](https://distrowatch.com/table.php?distribution=deepin) or another Debian-based distro that provides dde-file-manager:

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

### Build Debian package

A debian folder is provided to build the package under the deepin linux desktop distribution. To build the package, use the following command:

```shell
$ dpkg-buildpackage -uc -us -nc -b # build binary package(s)
```

## Usage

Execute `dde-file-manager`

## Documentation

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
