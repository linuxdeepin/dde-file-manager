##dde-file-manager 

Deepin File Manager is a file management tool independently  developed by Deepin Technology, featured with searching, copying, trash, compression/decompression, file property and other file management functions.

### Dependencies

### Build dependencies

* qmake (>= 5.3)
* gtk+-2.0 
* xcb 
* xcb-ewmh 
* libgsettings-qt 
* libudev 
* xext 
* libsecret-1 
* gio-unix-2.0
* dtkbase 
* dtkutil 
* dtkwidget
* libpoppler-cpp0v5
* libffmpegthumbnailer4v5


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

## Getting help
* [Official Forum](https://bbs.deepin.org/)
* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC Channel](https://webchat.freenode.net/?channels=deepin)
* [Wiki](http://wiki.deepin.org/)

## License

dde-file-manager is licensed under [GPLv3](LICENSE)
