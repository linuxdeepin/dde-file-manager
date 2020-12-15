#!/bin/bash

# 获取参数
CLEAR_BUILD="clear" #是否清场 not_clear 就不清场
UT_PRJ_TYPE=${1} #运行UT类型 ut_all dde_file_manager dde-file-manager-lib dde-desktop dde-dock-plugins dde-file-manager-plugins dde-file-thumbnail-tool deepin-anything-server-plugins

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
if [ "$CLEAR_BUILD" != "not_clear" ] ; then
    rm -rf $BUILD_DIR
    echo "clear " $BUILD_DIR
fi
# 创建文件夹
mkdir -p $BUILD_DIR

# 打印当前目录，当前目录应当是 build-ut
echo `pwd`
echo "start dde-file-manager project all CI cases"

# 下面是UT相关的case
if [ ! -n "$UT_PRJ_TYPE" ] ; then
    echo "ut case type is all"
    ./all-ut-prj-running.sh $PROJECT_FOLDER $BUILD_DIR "ut_all"
else
    echo "ut case type is " $UT_PRJ_TYPE
    ./all-ut-prj-running.sh $PROJECT_FOLDER $BUILD_DIR $UT_PRJ_TYPE
fi

exit 0
