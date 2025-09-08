# DFM Extension Example

`dfm-extension-example` 项目提供了一个基于文件管理器(dde-file-manager)扩展开发库(dfm-extension)的示例程序。

## 依赖

查看 `debian/control` 获取构建时和运行时的依赖，或使用 `cmake` 检查缺少的必需依赖。

快速开始，您至少需要安装：
```shell
$ sudo apt install libdfm-extension-dev 
```

## 构建

常规的 CMake 构建步骤如下：

```shell
$ cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
$ cmake --build build
```

构建完成后，您可以通过以下命令安装：

```shell
$ sudo cmake --build build --target install
```

项目提供了 `debian` 文件夹，用于在 _deepin_ Linux 桌面发行版下构建软件包。要构建软件包，请使用以下命令：

```shell
$ sudo apt build-dep . # 安装构建依赖
$ dpkg-buildpackage -uc -us -nc -b # 构建二进制包
```

## 参与贡献

* 通过 GitHub 贡献代码
* 在 GitHub Issues 或 GitHub Discussions 提交错误报告或建议

