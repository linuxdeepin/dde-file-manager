#!/bin/bash
build_dir=$(cd ../$(dirname $0)/build-test-filemanager-unknown-Debug; pwd)

./third-party/test-prj-running.sh $build_dir/dde-file-manager-lib test-dde-file-manager-lib

exit 0
