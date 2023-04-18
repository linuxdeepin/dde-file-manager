#!/bin/bash

# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set -x
export DISPLAY=":1"
export QT_QPA_PLATFORM=

# 定位脚本所在父目录
PROJECT_FOLDER=${1}

TESTS_FOLDER=$PROJECT_FOLDER/tests
SRC_FOLDER=$PROJECT_FOLDER/src

echo $TESTS_FOLDER
echo $SRC_FOLDER

# 定位build_ut
BUILD_DIR=${2}

UT_TESTS_FOLDER=$BUILD_DIR/tests
UT_SRC_FOLDER=$BUILD_DIR/src

echo $UT_SRC_FOLDER

#运行UT类型 all,后续需要补充
UT_PRJ_TYPE=${3}
UT_TYPE_ALL="all"
UT_TYPE_FILE_MANAGER="dde-file-manager"

REBUILD_PRJ=${4}
REBUILD_TYPE_YES="yes"

#qmake 参数
QMAKE_ARGS="-spec linux-g++ CONFIG+=debug"
#CPU 个数
CPU_NUMBER=${5}
#是否显示报告
SHOW_REPORT=${6}

check_ut_result()
{
  if [ $1 != 0 ]; then
     echo "Error: UT process is broken by: " $2 ",end with: "$1
     exit $1
  fi
}

# 打印当前目录，当前目录应当是 build-ut
echo `pwd`
echo "start dde-file-manager all UT cases:" $UT_PRJ_TYPE

# 下面是编译和工程测试
# 1. 编译工程
mkdir -p $UT_TESTS_FOLDER
cd $UT_TESTS_FOLDER
cmake $PROJECT_FOLDER -DCMAKE_BUILD_TYPE=Debug
make -j$CPU_NUMBER

regTestFolder()
{
  RELATIVEPATH=$1
  FOLDERNAME=${RELATIVEPATH##*/}

  if [ "$UT_PRJ_TYPE" = "$UT_TYPE_ALL" ] || [ "$UT_PRJ_TYPE" = "$FOLDERNAME" ] ; then
    echo "$FOLDERNAME test case is running"

    DIR_TEST=$UT_TESTS_FOLDER/tests/$RELATIVEPATH

    cd $DIR_TEST

    extract_path="*/src/$RELATIVEPATH/*  */include/dfm-extension/*  */include/dfm-framework/*"
    remove_path="*/third-party/* *tests* */build-ut/* *moc_* *qrc_*"
    # report的文件夹，报告后缀名，编译路径，可执行程序名，正向解析设置，逆向解析设置
    $TESTS_FOLDER/ut-target-running.sh $BUILD_DIR $FOLDERNAME $DIR_TEST test-$FOLDERNAME "$extract_path" "$remove_path" $SHOW_REPORT
    check_ut_result $? $FOLDERNAME
  fi
}

# 注册列表，测试目录放入该列表
regList=(
# apps
  # apps/dde-file-manager
  # apps/dde-desktop
  # apps/dde-file-manager-daemon
  # apps/dde-file-manager-server

# base、extension、framework
  dfm-base
  # dfm-extension
  dfm-framework

  # plugins/common/dfmplugin-burn

  # plugins/filemanager/core/dfmplugin-computer
  plugins/filemanager/core/dfmplugin-sidebar
  # plugins/filemanager/dfmplugin-myshares
  # plugins/filemanager/dfmplugin-smbbrowser
  plugins/filemanager/dfmplugin-optical

# services
  # services/dfm-business-services/dfm-desktop-service
  # services/dfm-business-services/dfm-filemanager-service
  # services/dfm-common-service
) 

# test prepare
echo "recompile gsettings for test"
mkdir -p $BUILD_DIR/glib-2.0/schemas
for f in $(find $PROJECT_FOLDER -name "*.xml");do cp $f $BUILD_DIR/glib-2.0/schemas/;done
glib-compile-schemas $BUILD_DIR/glib-2.0/schemas
export XDG_DATA_DIRS=$BUILD_DIR:$XDG_DATA_DIRS

# register test folders.
for((i=0; i<${#regList[@]}; i++)) 
do
FOLDER=${regList[i]}
echo "register test folder:$FOLDER"
regTestFolder $FOLDER
done;

echo "end dde-file-manager all UT cases"

exit 0
