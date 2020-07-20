#!/bin/bash
build_dir=$(cd ../$(dirname $0)/build-test-filemanager-unknown-Debug; pwd)

./third-party/test-prj-running.sh $build_dir/dde-file-manager-lib test-dde-file-manager-lib
./third-party/test-prj-running.sh $build_dir/dde-desktop test-dde-desktop
./third-party/test-prj-running.sh $build_dir/dde-dock-plugins/disk-mount test-dde-disk-mount-plugin
exit 0
