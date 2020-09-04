#!/bin/bash
build_dir=$(cd ../$(dirname $0)/build-test-dde-file-manager-lib-unknown-Debug; pwd)

#下面是覆盖率目录操作，一种正向操作，一种逆向操作，extract_path 是当前期望做覆盖率的目录;remove_path 是不期望做覆盖率的目录。注意：每个项目可以不同，试情况而定，下面是通用设置
extract_path="*/dde-file-manager-lib/*"
remove_path="*/tests/* */3rdParty/* */dde-file-manager-lib/vault/openssl/* */dde-file-manager-lib/vault/qrencode/*"

#dde-dock-plugins/disk-mount 测试报告
./../third-party/test-prj-running.sh $build_dir test-dde-file-manager-lib "$extract_path" "$remove_path"


exit 0
