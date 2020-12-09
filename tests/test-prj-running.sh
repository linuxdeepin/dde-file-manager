#!/bin/bash

# 定位脚本所在当前目录
TESTS_FOLDER=$(cd "$(dirname "$0")";pwd)
echo $TESTS_FOLDER

# 定位脚本所在父目录
PROJECT_FOLDER=$(cd "$(dirname "$TESTS_FOLDER")";pwd)
echo $PROJECT_FOLDER

# 定位脚本所在父目录下的某文件
BUILD_DIR=$PROJECT_FOLDER/build-ut
echo $BUILD_DIR

# 清场,删除编译文件夹
rm -rf $BUILD_DIR
# 创建文件夹
mkdir -p $BUILD_DIR

# 进入build-ut 目录
cd $BUILD_DIR

# 打印当前目录，当前目录应当是 build-ut
echo `pwd`

# 编译测试工程
qmake ../test-filemanager.pro
make -j4

#下面是覆盖率目录操作，一种正向操作，一种逆向操作，extract_path 是当前期望做覆盖率的目录;remove_path 是不期望做覆盖率的目录。注意：每个项目可以不同，试情况而定，下面是通用设置

# 1. 子项目 dde-dock-plugins/disk-mount 单元测试与覆盖率测试
disk_mount_extract_path="*/dde-dock-plugins/*"
disk_mount_remove_path="*/third-party/* *tests* *moc_* *qrc_*"
# report的文件夹，报告后缀名，编译路径，可执行程序名，正向解析设置，逆向解析设置
./../tests/target-running.sh $BUILD_DIR disk-mount $BUILD_DIR/dde-dock-plugins/disk-mount test-dde-disk-mount-plugin "$disk_mount_extract_path" "$disk_mount_remove_path"


exit 0
