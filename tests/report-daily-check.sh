#!/bin/bash

# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# 拉最新代码
git pull

#部署Python依赖
# sudo apt install python-pip python-bs4 python-lxml python-html5lib python3-bs4 lcov

# 定位脚本所在当前目录
TESTS_FOLDER=$(cd "$(dirname "$0")";pwd)
echo $TESTS_FOLDER

# 定位脚本所在父目录
PROJECT_FOLDER=$(cd "$(dirname "$TESTS_FOLDER")";pwd)
echo $PROJECT_FOLDER

# 定位脚本所在父目录下的某文件
BUILD_DIR=$PROJECT_FOLDER/build-ut
echo $BUILD_DIR

# 记录日志信息
DAILY_REPORT_DIR=$TESTS_FOLDER/daily-report
# 创建文件夹
mkdir -p $DAILY_REPORT_DIR

# 跑用例
./test-prj-running.sh

# 用例分析脚本
python report-daily-check.py $DAILY_REPORT_DIR $BUILD_DIR

exit 0