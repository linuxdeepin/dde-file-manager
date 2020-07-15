#!/bin/bash


#workdir=$(cd ../../$(dirname $0)/build-filemanager-unknown-Debug/test; pwd)
workdir=${1} #编译路径
executable=${2} #可执行程序的文件名

if [ ! $# -eq 2 ]; then
  echo "no project dir, usage: <编译路径> <可执行程序>"
  exit 0
fi

build_dir=$workdir
result_coverage_dir=$build_dir/coverage
result_report_dir=$build_dir/report/report.xml

#下面二选1进行操作，一种正向操作，一种逆向操作
extract_info="*/dde-file-manager/*"
remove_info="*/tests/* */third-party/*"

$build_dir/$executable --gtest_output=xml:$result_report_dir


lcov -d $build_dir -c -o $build_dir/coverage.info

lcov --remove $build_dir/coverage.info $remove_info --output-file $build_dir/coverage.info
lcov --extract $build_dir/coverage.info $extract_info --output-file  $build_dir/coverage.info

lcov --list-full-path -e $build_dir/coverage.info –o $build_dir/coverage-stripped.info

genhtml -o $result_coverage_dir $build_dir/coverage.info

nohup x-www-browser $result_coverage_dir/index.html &
nohup x-www-browser $result_report_dir &
 
lcov -d $build_dir –z

exit 0
