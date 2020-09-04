#!/bin/bash
build_dir=$(cd ../$(dirname $0)/build-test-filemanager-unknown-Debug; pwd)

#下面是覆盖率目录操作，一种正向操作，一种逆向操作，extract_path 是当前期望做覆盖率的目录;remove_path 是不期望做覆盖率的目录。注意：每个项目可以不同，试情况而定，下面是通用设置
extract_path="*/dde-file-manager/*"
remove_path="*/third-party/* *tests* */dde-file-manager-lib/vault/openssl/* */dde-file-manager-lib/vault/qrencode/*"

#dde-file-manager-lib 测试报告
./third-party/test-prj-running.sh $build_dir/dde-file-manager-lib test-dde-file-manager-lib "$extract_path" "$remove_path"

#dde-desktop 测试报告
#./third-party/test-prj-running.sh $build_dir/dde-desktop test-dde-desktop "$extract_path" "$remove_path"

#dde-dock-plugins/disk-mount 测试报告
cp dde-dock-plugins/test-dde-disk-mount-plugin $build_dir/dde-dock-plugins/disk-mount
./third-party/test-prj-running.sh $build_dir/dde-dock-plugins/disk-mount test-dde-disk-mount-plugin "$extract_path" "$remove_path"

#dde-file-manager-daemon 测试报告
./third-party/test-prj-running.sh $build_dir/dde-file-manager-daemon test-dde-file-manager-daemon "$extract_path" "$remove_path"

#dde-file-manager-plugins 测试报告
./third-party/test-prj-running.sh $build_dir/dde-file-manager-plugins/pluginPreview/dde-image-preview-plugin test-dde-image-preview-plugin "$extract_path" "$remove_path"
./third-party/test-prj-running.sh $build_dir/dde-file-manager-plugins/pluginPreview/dde-music-preview-plugin test-dde-music-preview-plugin "$extract_path" "$remove_path"
./third-party/test-prj-running.sh $build_dir/dde-file-manager-plugins/pluginPreview/dde-pdf-preview-plugin test-dde-pdf-preview-plugin "$extract_path" "$remove_path"
./third-party/test-prj-running.sh $build_dir/dde-file-manager-plugins/pluginPreview/dde-text-preview-plugin test-dde-text-preview-plugin "$extract_path" "$remove_path"
./third-party/test-prj-running.sh $build_dir/dde-file-manager-plugins/pluginPreview/dde-video-preview-plugin test-dde-video-preview-plugin "$extract_path" "$remove_path"

#dde-file-thumbnail-tool 测试报告
./third-party/test-prj-running.sh $build_dir/dde-file-thumbnail-tool/video test-video "$extract_path" "$remove_path"

#deepin-anything-server-plugins 测试报告
./third-party/test-prj-running.sh $build_dir/deepin-anything-server-plugins/dde-anythingmonitor/ test-dde-anythingmonitor "$extract_path" "$remove_path"


exit 0
