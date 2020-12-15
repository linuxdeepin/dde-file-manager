#!/bin/bash

# 定位脚本所在父目录
PROJECT_FOLDER=${1}
echo $PROJECT_FOLDER

# 定位脚本所在父目录下的某文件
BUILD_DIR=${2}

# 打印当前目录，当前目录应当是 build-ut
echo `pwd`
echo "start dde-file-manager all UT cases"

# 下面是编译和工程测试

# 1. 编译test-dde-file-manager-lib工程
DIR_TEST_DDE_FILE_MANAGER_LIB=$BUILD_DIR/test-dde-file-manager-lib
mkdir -p $DIR_TEST_DDE_FILE_MANAGER_LIB
cd $DIR_TEST_DDE_FILE_MANAGER_LIB
qmake $PROJECT_FOLDER/dde-file-manager-lib/test-dde-file-manager-lib.pro
make -j4

extract_path_dde_file_manager_lib="*/dde-file-manager-lib/*"
remove_path_dde_file_manager_lib="*/tests/* */3rdParty/* */dde-file-manager-lib/vault/openssl/* */dde-file-manager-lib/vault/qrencode/*"

./../../tests/ut-target-running.sh $BUILD_DIR dde-file-manager-lib $DIR_TEST_DDE_FILE_MANAGER_LIB test-dde-file-manager-lib "$extract_path_dde_file_manager_lib" "$remove_path_dde_file_manager_lib"

# 2 编译lib库用于支持其他项目
DIR_DDE_FILE_MANAGER_LIB=$BUILD_DIR/dde-file-manager-lib
mkdir -p $DIR_DDE_FILE_MANAGER_LIB
cd $DIR_DDE_FILE_MANAGER_LIB
qmake $PROJECT_FOLDER/dde-file-manager-lib/dde-file-manager-lib.pro
make -j4

# 3 编译并测试dde-desktop/test-dde-desktop
DIR_TEST_DDE_DESKTOP=$BUILD_DIR/dde-desktop
mkdir -p $DIR_TEST_DDE_DESKTOP
cd $DIR_TEST_DDE_DESKTOP
qmake $PROJECT_FOLDER/dde-desktop/test-dde-desktop.pro
make -j4

dde_desktop_extract_path="*/dde-desktop/*"
dde_desktop_remove_path="*/third-party/* *tests* *moc_* *qrc_*"
# report的文件夹，报告后缀名，编译路径，可执行程序名，正向解析设置，逆向解析设置
./../../tests/ut-target-running.sh $BUILD_DIR dde-desktop $DIR_TEST_DDE_DESKTOP test-dde-desktop "$dde_desktop_extract_path" "$dde_desktop_remove_path"


# 4. 子项目 dde-dock-plugins/disk-mount 单元测试与覆盖率测试
DIR_TEST_DDE_DOCK=$BUILD_DIR/dde-dock-plugins
mkdir -p $DIR_TEST_DDE_DOCK
cd $DIR_TEST_DDE_DOCK
qmake $PROJECT_FOLDER/dde-dock-plugins/test-dde-dock-plugins.pro
make -j4
disk_mount_extract_path="*/dde-dock-plugins/*"
disk_mount_remove_path="*/third-party/* *tests* *moc_* *qrc_*"
# report的文件夹，报告后缀名，编译路径，可执行程序名，正向解析设置，逆向解析设置
./../../tests/ut-target-running.sh $BUILD_DIR disk-mount $DIR_TEST_DDE_DOCK/disk-mount test-dde-disk-mount-plugin "$disk_mount_extract_path" "$disk_mount_remove_path"

echo "end dde-file-manager all UT cases"

exit 0
