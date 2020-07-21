#!/bin/bash
build_dir=$(cd ../$(dirname $0)/build-test-filemanager-unknown-Debug; pwd)

#下面是覆盖率目录操作，一种正向操作，一种逆向操作，extract_info 是当前期望做覆盖率的目录;remove_info 是不期望做覆盖率的目录
extract_info="*/dde-file-manager/*"
remove_info="*/dde-dock-plugins/* */third-party/*"

#dde-file-manager-lib 测试报告
./third-party/test-prj-running.sh $build_dir/dde-file-manager-lib test-dde-file-manager-lib extract_info remove_info

#dde-desktop 测试报告
./third-party/test-prj-running.sh $build_dir/dde-desktop test-dde-desktop extract_info remove_info

#dde-dock-plugins/disk-mount 测试报告
cp dde-dock-plugins/test-dde-disk-mount-plugin $build_dir/dde-dock-plugins/disk-mount
./third-party/test-prj-running.sh $build_dir/dde-dock-plugins/disk-mount test-dde-disk-mount-plugin extract_info remove_info


exit 0
