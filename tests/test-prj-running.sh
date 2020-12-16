#!/bin/bash

# 获取参数
CLEAR_COMMAND="yes"; #是否清场 no 就不清场
UT_COMMAND=""; #运行UT类型 no all dde_file_manager dde-file-manager-lib dde-desktop dde-dock-plugins dde-file-manager-plugins dde-file-thumbnail-tool deepin-anything-server-plugins
CPP_CHECK_COMMAND="yes"; #是否运行cppcheck，no就不运行

while [ $# -ge 2 ] ; do
        case "$1" in
                --clear) CLEAR_COMMAND="$2"; shift 2;;
                --ut) UT_COMMAND="$2"; shift 2;;
                --cppcheck) CPP_CHECK_COMMAND="$2"; shift 2;;
                *) echo "Unknown command parameter $1 $2, break the process!" ; exit 0; break;;
        esac
done

echo $CLEAR_COMMAND
echo $UT_COMMAND
echo $CPP_CHECK_COMMAND

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
if [ "$CLEAR_COMMAND" = "yes" ] ; then
    rm -rf $BUILD_DIR
    echo "clear " $BUILD_DIR
fi
# 创建文件夹
mkdir -p $BUILD_DIR

# 打印当前目录，当前目录应当是 build-ut
echo `pwd`
echo "start dde-file-manager project all CI cases"

# 下面是UT相关的case
if [ ! -n "$UT_COMMAND" ] ; then
    echo "ut case type is all"
    ./all-ut-prj-running.sh $PROJECT_FOLDER $BUILD_DIR "all" $CLEAR_COMMAND
elif [ $UT_COMMAND != "no" ] ; then
    echo "ut case type is " $UT_COMMAND
    ./all-ut-prj-running.sh $PROJECT_FOLDER $BUILD_DIR $UT_COMMAND $CLEAR_COMMAND
fi

# 下面是cppcheck相关case
if [ ! -n "$CPP_CHECK_COMMAND" ] || [ "$CPP_CHECK_COMMAND" = "yes" ] ; then
    echo "cppcheck case"
    ./cppcheck-prj-running.sh $PROJECT_FOLDER $BUILD_DIR
fi

exit 0
