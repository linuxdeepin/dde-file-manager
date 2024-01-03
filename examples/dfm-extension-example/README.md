# dfm-extension-example

一个基于文件管理器(dde-file-manager)的扩展开发库(dfm-extension)的示例程序。

## 依赖

```shell
$ sudo apt install libdfm-extension-dev 
```

## 安装

```shell
$ cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
$ cmake --build build
$ sudo cmake --build build --target install
```

安装后，重启生效。