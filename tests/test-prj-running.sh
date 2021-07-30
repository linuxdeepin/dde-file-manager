#!/bin/bash

export DISPLAY=":0"
export QT_QPA_PLATFORM=

#get --help
for argcommand in $*
do
  if [ "$argcommand" = "--help" ] ; then
        echo "--clear:默认情况为yes删除当前build-ut下所有数据， 使用 --clear no 不进行清除操作"
        echo "--rebuild：默认为yes进行重新编译， 使用--rebuild no 不进行重新编译"
        echo "--ut:指定ut项目类型，可以使用类型：no all dde_file_manager dde-file-manager-lib dde-desktop dde-dock-plugins dde-file-manager-plugins dde-file-thumbnail-tool deepin-anything-server-plugins"
        echo "--cppcheck：默认为no 不进行项目cpp-check文件扫描， 使用--cppcheck yes 进行cpp-check扫描"
        echo "--show：默认为no 不显示报表， 使用--show yes 显示当前报表"
        echo "--cpu：当前使用CPU数目，默认为16"
        exit 0
  fi
done

# 获取参数

CLEAR_COMMAND="yes"; #是否清场 no 就不清场
UT_COMMAND="all"; #运行UT类型 no all dde_file_manager dde-file-manager-lib dde-desktop dde-dock-plugins dde-file-manager-plugins dde-file-thumbnail-tool deepin-anything-server-plugins
REBUILD_PTJ="yes";
CPP_CHECK_COMMAND="no"; #是否运行cppcheck，no就不运行
CPU_NUMBER=16; #当前使用CPU数目，默认为16
SHOW_REPORT="no"; #默认为no 不显示报表

while [ $# -ge 2 ] ; do
        case "$1" in
                --clear) CLEAR_COMMAND="$2"; shift 2;;
                --ut) UT_COMMAND="$2"; shift 2;;
                --rebuild) REBUILD_PTJ="$2"; shift 2;;
                --cppcheck) CPP_CHECK_COMMAND="$2"; shift 2;;
                --cpu) CPU_NUMBER="$2"; shift 2;;
                --show) SHOW_REPORT="$2"; shift 2;;
                *) echo "Unknown command parameter $1 $2, break the process!" ; exit 0; break;;
        esac
done

echo "get command: " "--clear "$CLEAR_COMMAND "--ut "$UT_COMMAND "--rebuild "$REBUILD_PTJ "--cppcheck "$CPP_CHECK_COMMAND "--cpu "$CPU_NUMBER "--show "$SHOW_REPORT

check_process_result()
{
  if [ $1 != 0 ]; then
     echo "Error: CI is broken by: " $2  ",end with: " $1
     exit $1
  fi
}

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
    ./all-ut-prj-running.sh $PROJECT_FOLDER $BUILD_DIR "all" $REBUILD_PTJ $CPU_NUMBER $SHOW_REPORT
elif [ $UT_COMMAND != "no" ] ; then
    echo "ut case type is " $UT_COMMAND
    ./all-ut-prj-running.sh $PROJECT_FOLDER $BUILD_DIR $UT_COMMAND $REBUILD_PTJ $CPU_NUMBER $SHOW_REPORT
fi

check_process_result $? "UT sub process"

# 下面是cppcheck相关case
if [ ! -n "$CPP_CHECK_COMMAND" ] || [ "$CPP_CHECK_COMMAND" = "yes" ] ; then
    echo "cppcheck case"
    ./cppcheck-prj-running.sh $PROJECT_FOLDER $BUILD_DIR
fi

exit 0
