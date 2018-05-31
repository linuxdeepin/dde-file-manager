## dde-file-manager

Deepin File Manager is a file management tool independently developed by Deepin Technology, featured with searching, copying, trash, compression/decompression, file property and other file management functions.

### Dependencies

### Build dependencies
* pkg-config
* libgtk2.0-dev
* libglib2.0-dev
* libxcb-dev
* libxcb-ewmh-dev
* libxcb-util0-dev
* libx11-dev
* libgsettings-qt-dev
* libsecret-1-dev
* libpoppler-cpp-dev
* libffmpegthumbnailer-dev
* libpolkit-agent-1-dev
* libpolkit-qt5-1-dev
* Qt5(>= 5.6) with modules:
  - qt5-default
  - qt5-qmake
  - libqt5svg5-dev
  - qttools5-dev-tools
* Deepin-tool-kit(>=2.0) with modules:
  - dtkcore-dev
  - dtkwidget-dev


## Installation

### Build from source code

1. Make sure you have installed all dependencies.

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
 - [User Documentation](https://wiki.deepin.org/index.php?title=%E6%B7%B1%E5%BA%A6%E6%96%87%E4%BB%B6%E7%AE%A1%E7%90%86%E5%99%A8) (中文)

## Getting help

 - [Official Forum](https://bbs.deepin.org/)
 - [Developer Center](https://github.com/linuxdeepin/developer-center)
 - [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
 - [IRC Channel](https://webchat.freenode.net/?channels=deepin)
 - [Wiki](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

 - [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
 - [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License

dde-file-manager is licensed under [GPLv3](LICENSE)
