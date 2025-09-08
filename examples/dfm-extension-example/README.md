# DFM Extension Example

The `dfm-extension-example` project provides an example program based on the file manager (dde-file-manager) extension development library (dfm-extension).

## Dependencies

Check `debian/control` for build-time and runtime dependencies, or use `cmake` to check the missing required dependencies.

For a quick start, you need at least:
```shell
$ sudo apt install libdfm-extension-dev 
```

## Building

Regular CMake building steps applies, in short:

```shell
$ cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
$ cmake --build build
```

After building, you can install it by:

```shell
$ sudo cmake --build build --target install
```

A `debian` folder is provided to build the package under the _deepin_ linux desktop distribution. To build the package, use the following command:

```shell
$ sudo apt build-dep . # install build dependencies
$ dpkg-buildpackage -uc -us -nc -b # build binary package(s)
```

## Getting Involved

* Code contribution via GitHub
* Submit bug or suggestions to GitHub Issues or GitHub Discussions
