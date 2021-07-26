#!/bin/bash

export DISPLAY=":0"
export QT_QPA_PLATFORM=

# cppcheck检测目录
SRC_DIR=${1}/src
echo $SRC_DIR

# build-ut目录
BUILD_DIR=${2}

# 打印当前目录，当前目录应当是 build-ut
echo `pwd`
echo "start dde-file-manager cppcheck case"

# 下面是cppcheck检测
# report目录
REPORT_FILE_PATH=$BUILD_DIR/cppcheck_dde-file-manager.xml
# 打印cppcheck命令
echo "cppcheck --std=c++11 --language=c++ --enable=all --output-file="$REPORT_FILE_PATH "--xml" $SRC_DIR
# 执行cppcheck命令
cppcheck --std=c++11 --language=c++ --enable=all --output-file=$REPORT_FILE_PATH --xml $SRC_DIR

echo "end dde-file-manager cppcheck case"

exit 0
