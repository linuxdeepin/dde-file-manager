# dde-file-manager

深度文件管理器是由深度公司自主研发，具有文件搜索、复制、文件回收站、文件压缩/解压、文件属性等功能的文件管理工具。

### 依赖

### 构建依赖

_当前的开发分支为**master**，编译依赖可能会在没有更新本说明的情况下发生变化，请参考`./debian/control`以获取构建依赖项列表_

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

## 安装

### 构建过程

1. 确保已经安装所有依赖库。

   _不同发行版的软件包名称可能不同，如果您的发行版提供了dde-file-manager，请检查发行版提供的打包脚本。_

如果你使用的是[Deepin](https://distrowatch.com/table.php?distribution=deepin)或其他提供了文件管理器的基于Debian的发行版：

``` shell
$ apt build-dep dde-file-manager
```

2. 构建:
```
$ cd dde-file-manager
$ mkdir build
$ cd build
$ qmake ../filemanager.pro
$ make
```

3. 安装:
```
$ sudo make install
```

可执行程序为 `/usr/bin/dde-file-manager`

## 使用

执行 `dde-file-manager`

## 文档

- [开发文档](https://linuxdeepin.github.io/dde-file-manager/)
- [User Documentation](https://wiki.deepin.org/wiki/Deepin_File_Manager) | [用户文档](https://wiki.deepin.org/index.php?title=%E6%B7%B1%E5%BA%A6%E6%96%87%E4%BB%B6%E7%AE%A1%E7%90%86%E5%99%A8)

## 帮助

- [官方论坛](https://bbs.deepin.org/) 
- [开发者中心](https://github.com/linuxdeepin/developer-center) 
- [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
- [聊天室](https://webchat.freenode.net/?channels=deepin)
- [Wiki](https://wiki.deepin.org/)

## 贡献指南

我们鼓励您报告问题并做出更改

- [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) (English)

- [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)
- [在Transife上翻译您的语言](https://www.transifex.com/linuxdeepin/deepin-file-manager/)

## 开源许可证

dde-file-manager 在 [GPL-3.0-or-later](LICENSE.txt)下发布。
