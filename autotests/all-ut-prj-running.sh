#!/bin/bash

# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

export DISPLAY=":0"
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

# run one target and get the ut coverage
testTargetRun()
{
  RELATIVEPATH=$1
  EXTRACT_PATH=$2
  REMOVE_PATH=$3

  echo "EXTRACT_PATH filter are:$EXTRACT_PATH"
  echo "REMOVE_PATH filter are:$REMOVE_PATH"

  FOLDERNAME=${RELATIVEPATH##*/}

  if [ "$UT_PRJ_TYPE" = "$UT_TYPE_ALL" ] || [ "$UT_PRJ_TYPE" = "$FOLDERNAME" ] ; then
    echo "$FOLDERNAME test case is running"

    DIR_TEST=$UT_TESTS_FOLDER/tests/$RELATIVEPATH

    cd $DIR_TEST

    # report的文件夹，报告后缀名，编译路径，可执行程序名，正向解析设置，逆向解析设置
    $TESTS_FOLDER/ut-target-running.sh $BUILD_DIR $FOLDERNAME $DIR_TEST test-$FOLDERNAME "$EXTRACT_PATH" "$REMOVE_PATH" $SHOW_REPORT
    check_ut_result $? $FOLDERNAME
  fi
}

# 对外暴露框架、接口放入该列表
interfaceTestList=(

# base、extension、framework
  dfm-base
  # dfm-extension
  dfm-framework
) 

# App、业务插件，测试目录放入该列表
businessTestList=(
# apps
  # apps/dde-file-manager
    apps/dde-desktop
  # apps/dde-file-manager-daemon
  # apps/dde-file-manager-server

# plugins
  plugins/common/core/dfmplugin-bookmark
  plugins/common/core/dfmplugin-fileoperations
  plugins/common/dfmplugin-burn
  plugins/common/core/dfmplugin-trashcore
  plugins/common/dfmplugin-utils
  plugins/common/dfmplugin-tag
  plugins/common/dfmplugin-dirshare
  plugins/common/dfmplugin-preview/filepreview
  plugins/common/dfmplugin-preview/pluginpreviews/image-preview
  plugins/common/dfmplugin-preview/pluginpreviews/music-preview
#  plugins/common/dfmplugin-preview/pluginpreviews/pdf-preview
  plugins/common/dfmplugin-preview/pluginpreviews/text-preview
  plugins/common/dfmplugin-preview/pluginpreviews/video-preview

  plugins/filemanager/core/dfmplugin-workspace
  plugins/filemanager/core/dfmplugin-computer
  plugins/filemanager/core/dfmplugin-sidebar
  plugins/filemanager/core/dfmplugin-recent
  plugins/filemanager/core/dfmplugin-detailspace
  plugins/filemanager/dfmplugin-myshares
  plugins/filemanager/dfmplugin-smbbrowser
  plugins/filemanager/dfmplugin-optical
  plugins/filemanager/dfmplugin-vault
  plugins/filemanager/dfmplugin-search

  plugins/desktop/core/ddplugin-canvas
  plugins/desktop/core/ddplugin-core
  plugins/desktop/ddplugin-background
  plugins/desktop/ddplugin-wallpapersetting
  plugins/desktop/ddplugin-organizer
) 

# 2. interface test .
INTERFACE_REMOVE="*/3rdparty/* *tests* */build-ut/* *moc_* *qrc_*"

for((i=0; i<${#interfaceTestList[@]}; i++)) 
do
FOLDER=${interfaceTestList[i]}
echo "interface folder:$FOLDER"
INTERFACE_EXTRACT="*/src/$FOLDER/*  */include/dfm-extension/*  */include/dfm-framework/*"
testTargetRun $FOLDER "$INTERFACE_EXTRACT" "$INTERFACE_REMOVE"
done;

# 3. business test .
BUSINESS_REMOVE="*/3rdparty/* *tests* */build-ut/* *moc_* *qrc_* */dbus-private/*"

for((i=0; i<${#businessTestList[@]}; i++)) 
do
FOLDER=${businessTestList[i]}
echo "register test folder:$FOLDER"
BUSINESS_EXTRACT="*/src/$FOLDER/*"
testTargetRun $FOLDER "$BUSINESS_EXTRACT" "$BUSINESS_REMOVE"
done;

echo "end dde-file-manager all UT cases"

exit 0
