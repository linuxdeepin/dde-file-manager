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

# 打印当前目录，当前目录应当是 build-ut
echo `pwd`
echo "start dde-file-manager project all CI cases"

# 下面是UT相关的case

./all-ut-prj-running.sh $PROJECT_FOLDER $BUILD_DIR

exit 0
