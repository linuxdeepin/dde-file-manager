#!/bin/bash


#报告生成的地方
REPORT_DIR=${1}
#报告后缀名
REPORT_NAME=${2}
#编译路径
BUILD_DIR=${3}
#可执行程序的文件名
APP_NAME=${4}

#下面是覆盖率目录操作，一种正向操作，一种逆向操作
EXTRACT_INFO=${5} #针对当前目录进行覆盖率操作
REMOVE_INFO=${6} #排除当前目录进行覆盖率操作
SHOW_REPORT=${7} #是否显示报表

if [ ! $# -eq 7 ]; then
  echo "param number set err: "$# ": "$*
  echo "wrong param set,usage: <报告路径> <报告项目名> <编译路径> <可执行程序> <期望覆盖的路径> <不期望覆盖的路径>"
  exit 0
fi

RESULT_COVERAGE_DIR=$REPORT_DIR/html
#创建 coverage 总目录
mkdir -p $RESULT_COVERAGE_DIR

COVERAGE_INFO=$BUILD_DIR/covinfo_$REPORT_NAME.info

RESULT_UT_REPORT_DIR=$REPORT_DIR/report/report_$REPORT_NAME.xml

$BUILD_DIR/$APP_NAME --gtest_output=xml:$RESULT_UT_REPORT_DIR



lcov -d $BUILD_DIR -c -o $COVERAGE_INFO

lcov --extract $COVERAGE_INFO $EXTRACT_INFO --output-file  $COVERAGE_INFO
lcov --remove $COVERAGE_INFO $REMOVE_INFO --output-file $COVERAGE_INFO

lcov --list-full-path -e $COVERAGE_INFO –o $BUILD_DIR/coverage-stripped.info

genhtml -o $RESULT_COVERAGE_DIR $COVERAGE_INFO

mv $RESULT_COVERAGE_DIR/index.html $RESULT_COVERAGE_DIR/cov_$REPORT_NAME.html
mv $RESULT_COVERAGE_DIR/index-sort-f.html $RESULT_COVERAGE_DIR/index-sort-f_$REPORT_NAME.html
mv $RESULT_COVERAGE_DIR/index-sort-l.html $RESULT_COVERAGE_DIR/index-sort-l_$REPORT_NAME.html

if [ ! -n "$SHOW_REPORT" ] || [ "$SHOW_REPORT" = "yes" ] ; then
    nohup x-www-browser $RESULT_COVERAGE_DIR/cov_$REPORT_NAME.html &
    nohup x-www-browser $RESULT_UT_REPORT_DIR &
fi

lcov -d $BUILD_DIR –z

exit 0
