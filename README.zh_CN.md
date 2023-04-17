# dde-file-manager

深度文件管理器是由深度公司自主研发，具有文件搜索、复制、文件回收站、文件压缩/解压、查看文件属性等功能的文件管理工具。

### 依赖

### 构建依赖

_当前的开发分支为**master**，编译依赖可能会在没有更新本说明的情况下发生变化，请参考`./debian/control`以获取构建依赖项列表_

  * cmake,
* debhelper (>=9),
* pkg-config,
* dh-systemd,
* qtbase5-dev,
* qtbase5-private-dev,
* qtmultimedia5-dev,
* libffmpegthumbnailer-dev,
* libqt5svg5-dev,
* libpolkit-agent-1-dev, 
* libpolkit-qt5-1-dev,
* libdtkwidget-dev,
* libdtkgui-dev,
* libdtkcore-dev,
* libdtkcore5-bin,
* qttools5-dev-tools,
* dde-dock-dev(>=4.8.4.1),
* libdframeworkdbus-dev,
* libtag1-dev,
* libdmr-dev,
* libicu-dev,
* libdeepin-pdfium-dev,
* libqt5xdg-dev,
* libgio-qt-dev,
* libdfm-io-dev,
* libdfm-mount-dev,
* libdfm-burn-dev,
* libssl-dev,
* libgtest-dev,
* libgmock-dev,
* libgsettings-qt-dev,
* liblucene++-dev,
* libdocparser-dev,
* libboost-filesystem-dev,
* libsecret-1-dev,
* libkf5codecs-dev,
* libpoppler-cpp-dev,
* libcryptsetup-dev,
* deepin-desktop-base | deepin-desktop-server | deepin-desktop-device

## 安装

### 构建过程

1. 确保已经安装所有依赖库。

   _不同发行版的软件包名称可能不同，如果您的发行版提供了dde-file-manager，请检查发行版提供的打包脚本。_

如果你使用的是[Deepin](https://distrowatch.com/table.php?distribution=deepin)或其他提供了文件管理器的基于Debian的发行版：

``` shell
$ git clone https://github.com/linuxdeepin/dde-file-manager
$ cd dde-file-manager
$ sudo apt build-dep ./
```

2. 构建:
```shell
$ cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
$ cmake --build build
```

3. 安装:
```shell
$ sudo cmake --build build --target install
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
