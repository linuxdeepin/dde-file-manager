#!/bin/bash

export DISPLAY=":0"
export QT_QPA_PLATFORM=

# 定位脚本所在父目录
PROJECT_FOLDER=${1}

TESTS_FOLDER=$PROJECT_FOLDER/tests
SRC_FOLDER=$PROJECT_FOLDER/src

echo $TESTS_FOLDER
echo $SRC_FOLDER

# 定位build_ut
BUILD_DIR=${2}

UT_TESTS_FOLDER=$BUILD_DIR/tests
UT_SRC_FOLDER=$BUILD_DIR/src

echo $UT_SRC_FOLDER

#运行UT类型 all dde_file_manager dde-file-manager-lib dde-desktop dde-dock-plugins dde-file-manager-plugins dde-file-thumbnail-tool deepin-anything-server-plugins
UT_PRJ_TYPE=${3}
UT_TYPE_ALL="all"
UT_TYPE_FILE_MANAGER="dde_file_manager"
UT_TYPE_FILE_MANAGER_LIB="dde-file-manager-lib"
UT_TYPE_DDE_DESKTOP="dde-desktop"
UT_TYPE_DOCK_PLUGINS="dde-dock-plugins"
UT_TYPE_FILE_MANAGER_PLUGINS="dde-file-manager-plugins"
UT_TYPE_FILE_THUMBNAIL_TOOL="dde-file-thumbnail-tool"
UT_TYPE_ANYTHING_SERVER_PLUGINS="deepin-anything-server-plugins"
UT_TYPE_FILE_MANAGER_DAEMON="dde-file-manager-daemon"

REBUILD_PRJ=${4}
REBUILD_TYPE_YES="yes"

#qmake 参数
QMAKE_ARGS="-spec linux-g++ CONFIG+=debug"
#CPU 个数
CPU_NUMBER=${5}
#是否显示报告
SHOW_REPORT=${6}

check_ut_result()
{
  if [ $1 != 0 ]; then
     echo "Error: UT process is broken by: " $2 ",end with: "$1
     exit $1
  fi
}

# 打印当前目录，当前目录应当是 build-ut
echo `pwd`
echo "start dde-file-manager all UT cases:" $UT_PRJ_TYPE

# 下面是编译和工程测试

# 1. 编译test-dde-file-manager-lib工程
if [ "$UT_PRJ_TYPE" = "$UT_TYPE_ALL" ] || [ "$UT_PRJ_TYPE" = "$UT_TYPE_FILE_MANAGER_LIB" ] ; then
        echo $UT_TYPE_FILE_MANAGER_LIB "test case is running"

	DIR_TEST_DDE_FILE_MANAGER_LIB=$UT_TESTS_FOLDER/test-dde-file-manager-lib
	mkdir -p $DIR_TEST_DDE_FILE_MANAGER_LIB
	cd $DIR_TEST_DDE_FILE_MANAGER_LIB

        if [ "$REBUILD_PRJ" = "$REBUILD_TYPE_YES" ] ; then
                echo $UT_TYPE_FILE_MANAGER_LIB "is making"
		qmake $TESTS_FOLDER/dde-file-manager-lib/test-dde-file-manager-lib.pro $QMAKE_ARGS
		make -j$CPU_NUMBER
        fi

	extract_path_dde_file_manager_lib="*/src/dde-file-manager-lib/*"
	remove_path_dde_file_manager_lib="*/tests/* */3rdParty/* */dde-file-manager-lib/vault/openssl/* */build-ut/* */dde-file-manager-lib/vault/qrencode/*"
        # report的文件夹，报告后缀名，编译路径，可执行程序名，正向解析设置，逆向解析设置
	./../../../tests/ut-target-running.sh $BUILD_DIR dde-file-manager-lib $DIR_TEST_DDE_FILE_MANAGER_LIB test-dde-file-manager-lib "$extract_path_dde_file_manager_lib" "$remove_path_dde_file_manager_lib" $SHOW_REPORT
        check_ut_result $? $UT_TYPE_FILE_MANAGER_LIB
fi
# 2 编译lib库用于支持其他项目
NEED_LIB_DDE_FILE_MANAGER=true
if [ "$UT_PRJ_TYPE" = "$UT_TYPE_FILE_MANAGER_LIB" ] || [ "$UT_PRJ_TYPE" = "$UT_TYPE_DOCK_PLUGINS" ]; then
       NEED_LIB_DDE_FILE_MANAGER=false
       echo "set NEED_LIB_DDE_FILE_MANAGER as false to not support dde-file-manager-lib compilation"
fi

if [ "$NEED_LIB_DDE_FILE_MANAGER" = true ] ; then
        echo "common dde-file-manager lib is building for all sub projects"

	DIR_DDE_FILE_MANAGER_LIB=$UT_SRC_FOLDER/dde-file-manager-lib
	mkdir -p $DIR_DDE_FILE_MANAGER_LIB
	cd $DIR_DDE_FILE_MANAGER_LIB

        if [ "$REBUILD_PRJ" = "$REBUILD_TYPE_YES" ] ; then
                echo "common dde-file-manager lib is making"
		qmake $SRC_FOLDER/dde-file-manager-lib/dde-file-manager-lib.pro $QMAKE_ARGS
		make -j$CPU_NUMBER
        fi
        
fi

# dde-file-manager-lib 动态库依赖
export LD_LIBRARY_PATH=$UT_SRC_FOLDER/dde-file-manager-lib/:$LD_LIBRARY_PATH

# 3 编译并测试dde-desktop/test-dde-desktop
if [ "$UT_PRJ_TYPE" = "$UT_TYPE_ALL" ] || [ "$UT_PRJ_TYPE" = "$UT_TYPE_DDE_DESKTOP" ] ; then
        echo $UT_TYPE_DDE_DESKTOP "test case is running"

	DIR_TEST_DDE_DESKTOP=$UT_TESTS_FOLDER/dde-desktop
	mkdir -p $DIR_TEST_DDE_DESKTOP
	cd $DIR_TEST_DDE_DESKTOP

        if [ "$REBUILD_PRJ" = "$REBUILD_TYPE_YES" ] ; then
                echo $UT_TYPE_DDE_DESKTOP "is making"
		qmake $TESTS_FOLDER/dde-desktop/test-dde-desktop.pro $QMAKE_ARGS
		make -j$CPU_NUMBER
        fi

	dde_desktop_extract_path="*/dde-desktop/* */dde-wallpaper-chooser/*"
    dde_desktop_remove_path="*/third-party/* *tests* */dde-file-manager-lib/vault/openssl/* */dde-file-manager-lib/vault/qrencode/* */dde-desktop/dbus/* */build-ut/* */dde-wallpaper-chooser/dbus/*  *moc_* *qrc_*"
	# report的文件夹，报告后缀名，编译路径，可执行程序名，正向解析设置，逆向解析设置
	./../../../tests/ut-target-running.sh $BUILD_DIR dde-desktop $DIR_TEST_DDE_DESKTOP test-dde-desktop "$dde_desktop_extract_path" "$dde_desktop_remove_path" $SHOW_REPORT
        check_ut_result $? $UT_TYPE_DDE_DESKTOP
fi

# 4. 子项目 dde-dock-plugins/disk-mount 单元测试与覆盖率测试
if [ "$UT_PRJ_TYPE" = "$UT_TYPE_ALL" ] || [ "$UT_PRJ_TYPE" = "$UT_TYPE_DOCK_PLUGINS" ] ; then
	echo $UT_TYPE_DOCK_PLUGINS "test case is running"

        DIR_TEST_DDE_DOCK=$UT_TESTS_FOLDER/dde-dock-plugins
	mkdir -p $DIR_TEST_DDE_DOCK
	cd $DIR_TEST_DDE_DOCK

        if [ "$REBUILD_PRJ" = "$REBUILD_TYPE_YES" ] ; then
                echo $UT_TYPE_DOCK_PLUGINS "is making"
		qmake $TESTS_FOLDER/dde-dock-plugins/test-dde-dock-plugins.pro $QMAKE_ARGS
		make -j$CPU_NUMBER
        fi

	disk_mount_extract_path="*/src/dde-dock-plugins/*"
	disk_mount_remove_path="*/third-party/* *tests* */build-ut/* *moc_* *qrc_*"
	# report的文件夹，报告后缀名，编译路径，可执行程序名，正向解析设置，逆向解析设置
	./../../../tests/ut-target-running.sh $BUILD_DIR disk-mount $DIR_TEST_DDE_DOCK/disk-mount test-dde-disk-mount-plugin "$disk_mount_extract_path" "$disk_mount_remove_path" $SHOW_REPORT
        check_ut_result $? $UT_TYPE_DOCK_PLUGINS
fi

# 5. 子项目 dde-file-manager 单元测试与覆盖率测试
if [ "$UT_PRJ_TYPE" = "$UT_TYPE_ALL" ] || [ "$UT_PRJ_TYPE" = "$UT_TYPE_FILE_MANAGER" ] ; then
	echo $UT_TYPE_FILE_MANAGER "test case is running"

        DIR_TEST_DDE_FILE_MANAGER=$UT_TESTS_FOLDER/dde-file-manager
	mkdir -p $DIR_TEST_DDE_FILE_MANAGER
	cd $DIR_TEST_DDE_FILE_MANAGER

        if [ "$REBUILD_PRJ" = "$REBUILD_TYPE_YES" ] ; then
                echo $UT_TYPE_FILE_MANAGER "is making"
		qmake $TESTS_FOLDER/dde-file-manager/test-dde-file-manager.pro $QMAKE_ARGS
		make -j$CPU_NUMBER
        fi

	dde_file_manager_extract_path="*/src/dde-file-manager/* */src/utils/*"
	dde_file_manager_remove_path="*/third-party/* *tests* */build-ut/* */dde-file-manager-lib/interfaces/* */dde-desktop/dbus/* */dde-wallpaper-chooser/dbus/* *moc_* *qrc_*"
	# report的文件夹，报告后缀名，编译路径，可执行程序名，正向解析设置，逆向解析设置
	./../../../tests/ut-target-running.sh $BUILD_DIR file-manager $DIR_TEST_DDE_FILE_MANAGER test-file-manager "$dde_file_manager_extract_path" "$dde_file_manager_remove_path" $SHOW_REPORT
        check_ut_result $? $UT_TYPE_FILE_MANAGER
fi

# 6. 子项目 dde-file-manager-plugins 单元测试与覆盖率测试
if [ "$UT_PRJ_TYPE" = "$UT_TYPE_ALL" ] || [ "$UT_PRJ_TYPE" = "$UT_TYPE_FILE_MANAGER_PLUGINS" ] ; then
	echo $UT_TYPE_FILE_MANAGER_PLUGINS "test case is running"

        DIR_TEST_DDE_FILE_MANAGER_PLUGINS=$UT_TESTS_FOLDER/dde-file-manager-plugins
	mkdir -p $DIR_TEST_DDE_FILE_MANAGER_PLUGINS
	cd $DIR_TEST_DDE_FILE_MANAGER_PLUGINS

        if [ "$REBUILD_PRJ" = "$REBUILD_TYPE_YES" ] ; then
                echo $UT_TYPE_FILE_MANAGER_PLUGINS "is making"
		qmake $TESTS_FOLDER/dde-file-manager-plugins/test-dde-file-manager-plugins.pro $QMAKE_ARGS
		make -j$CPU_NUMBER
        fi

	dde_file_manager_plugins_extract_path="*/src/dde-file-manager-plugins/*"
	dde_file_manager_plugins_remove_path="*/third-party/* *tests* */dde-file-manager-lib/vault/openssl/* */dde-file-manager-lib/vault/qrencode/* */dde-desktop/dbus/* */dde-wallpaper-chooser/dbus/* *moc_* *qrc_* */dde-file-manager-plugins/plugininterfaces/view/*"
	# report的文件夹，报告后缀名，编译路径，可执行程序名，正向解析设置，逆向解析设置
    ./../../../tests/ut-target-running.sh $BUILD_DIR dde-image-preview-plugin $DIR_TEST_DDE_FILE_MANAGER_PLUGINS/pluginPreview/dde-image-preview-plugin test-dde-image-preview-plugin  "$dde_file_manager_plugins_extract_path" "$dde_file_manager_plugins_remove_path" $SHOW_REPORT
    check_ut_result $? $UT_TYPE_FILE_MANAGER_PLUGINS

    ./../../../tests/ut-target-running.sh $BUILD_DIR dde-music-preview-plugin $DIR_TEST_DDE_FILE_MANAGER_PLUGINS/pluginPreview/dde-music-preview-plugin test-dde-music-preview-plugin  "$dde_file_manager_plugins_extract_path" "$dde_file_manager_plugins_remove_path" $SHOW_REPORT
    check_ut_result $? $UT_TYPE_FILE_MANAGER_PLUGINS

    ./../../../tests/ut-target-running.sh $BUILD_DIR dde-pdf-preview-plugin $DIR_TEST_DDE_FILE_MANAGER_PLUGINS/pluginPreview/dde-pdf-preview-plugin test-dde-pdf-preview-plugin  "$dde_file_manager_plugins_extract_path" "$dde_file_manager_plugins_remove_path" $SHOW_REPORT
    check_ut_result $? $UT_TYPE_FILE_MANAGER_PLUGINS

    ./../../../tests/ut-target-running.sh $BUILD_DIR dde-text-preview-plugin $DIR_TEST_DDE_FILE_MANAGER_PLUGINS/pluginPreview/dde-text-preview-plugin test-dde-text-preview-plugin  "$dde_file_manager_plugins_extract_path" "$dde_file_manager_plugins_remove_path" $SHOW_REPORT
    check_ut_result $? $UT_TYPE_FILE_MANAGER_PLUGINS

    ./../../../tests/ut-target-running.sh $BUILD_DIR dde-video-preview-plugin $DIR_TEST_DDE_FILE_MANAGER_PLUGINS/pluginPreview/dde-video-preview-plugin test-dde-video-preview-plugin  "$dde_file_manager_plugins_extract_path" "$dde_file_manager_plugins_remove_path" $SHOW_REPORT
    check_ut_result $? $UT_TYPE_FILE_MANAGER_PLUGINS

    ./../../../tests/ut-target-running.sh $BUILD_DIR pluginView $DIR_TEST_DDE_FILE_MANAGER_PLUGINS/pluginView test-pluginView  "$dde_file_manager_plugins_extract_path" "$dde_file_manager_plugins_remove_path" $SHOW_REPORT
    check_ut_result $? $UT_TYPE_FILE_MANAGER_PLUGINS
fi

# 7. 子项目 dde-file-thumbnail-tool 单元测试与覆盖率测试
if [ "$UT_PRJ_TYPE" = "$UT_TYPE_ALL" ] || [ "$UT_PRJ_TYPE" = "$UT_TYPE_FILE_THUMBNAIL_TOOL" ] ; then
	echo $UT_TYPE_FILE_THUMBNAIL_TOOL "test case is running"

        DIR_TEST_DDE_FILE_THUMBNAIL=$UT_TESTS_FOLDER/dde-file-thumbnail-tool
	mkdir -p $DIR_TEST_DDE_FILE_THUMBNAIL
	cd $DIR_TEST_DDE_FILE_THUMBNAIL

        if [ "$REBUILD_PRJ" = "$REBUILD_TYPE_YES" ] ; then
                echo $UT_TYPE_FILE_THUMBNAIL_TOOL "is making"
		qmake $TESTS_FOLDER/dde-file-thumbnail-tool/test-dde-file-thumbnail-tool.pro $QMAKE_ARGS
		make -j$CPU_NUMBER
        fi

	dde_file_thumbnail_extract_path="*/src/dde-file-thumbnail-tool/*"
	dde_file_thumbnail_remove_path="*/third-party/* *tests* */dde-file-manager-lib/vault/openssl/* */dde-file-manager-lib/vault/qrencode/* */dde-desktop/dbus/* */dde-wallpaper-chooser/dbus/* *moc_* *qrc_*"
	# report的文件夹，报告后缀名，编译路径，可执行程序名，正向解析设置，逆向解析设置
	./../../../tests/ut-target-running.sh $BUILD_DIR dde-file-thumbnail-tool $DIR_TEST_DDE_FILE_THUMBNAIL test-video "$dde_file_thumbnail_extract_path" "$dde_file_thumbnail_remove_path" $SHOW_REPORT
        check_ut_result $? $UT_TYPE_FILE_THUMBNAIL_TOOL
fi

# aarch64 sw_64 mips64 mips32不支持deepin-anything
hw_arrch=$(uname -m)

# 8. 子项目 dde-file-manager-daemon中的accesscontrol 单元测试与覆盖率测试
if ([ "$hw_arrch" != "aarch64" ]) && ([ "$UT_PRJ_TYPE" = "$UT_TYPE_ALL" ] || [ "$UT_PRJ_TYPE" = "$UT_TYPE_FILE_MANAGER_DAEMON" ]) ; then
        echo $UT_TYPE_FILE_MANAGER_DAEMON "test case is running"

        DIR_TEST_DDE_FILE_MANAGER_DAEMON=$UT_TESTS_FOLDER/dde-file-manager-daemon
        mkdir -p $DIR_TEST_DDE_FILE_MANAGER_DAEMON
        cd $DIR_TEST_DDE_FILE_MANAGER_DAEMON

        if [ "$REBUILD_PRJ" = "$REBUILD_TYPE_YES" ] ; then
                echo $UT_TYPE_FILE_MANAGER_DAEMON "is making"
                qmake $TESTS_FOLDER/dde-file-manager-daemon/test-dde-file-manager-daemon.pro $QMAKE_ARGS
                make -j$CPU_NUMBER
        fi

        dde_file_manager_daemon_extract_path="*/src/dde-file-manager-daemon/accesscontrol/*"
        dde_file_manager_daemon_remove_path="*/third-party/* *tests* */dde-file-manager-lib/vault/openssl/* */dde-file-manager-lib/vault/qrencode/* */dde-desktop/dbus/* */dde-wallpaper-chooser/dbus/* *moc_* *qrc_*"
        # report的文件夹，报告后缀名，编译路径，可执行程序名，正向解析设置，逆向解析设置
        ./../../../tests/ut-target-running.sh $BUILD_DIR dde-file-manager-daemon $DIR_TEST_DDE_FILE_MANAGER_DAEMON test-dde-file-manager-daemon "$dde_file_manager_daemon_extract_path" "$dde_file_manager_daemon_remove_path" $SHOW_REPORT
        check_ut_result $? $UT_TYPE_FILE_MANAGER_DAEMON
fi

# 9. 子项目 deepin-anything-server-plugins 单元测试与覆盖率测试
if ([ "$hw_arrch" != "aarch64" ] && [ "$hw_arrch" != "sw_64" ] && [ "$hw_arrch" != "mips64" ] && [ "$hw_arrch" != "mips32" ]) &&
    ([ "$UT_PRJ_TYPE" = "$UT_TYPE_ALL" ] || [ "$UT_PRJ_TYPE" = "$UT_TYPE_ANYTHING_SERVER_PLUGINS" ]); then
	echo $UT_TYPE_ANYTHING_SERVER_PLUGINS "test case is running"

        DIR_TEST_DEEPIN_ANYTHING_SERVER_PLUGINS=$UT_TESTS_FOLDER/deepin-anything-server-plugins
	mkdir -p $DIR_TEST_DEEPIN_ANYTHING_SERVER_PLUGINS
	cd $DIR_TEST_DEEPIN_ANYTHING_SERVER_PLUGINS

        if [ "$REBUILD_PRJ" = "$REBUILD_TYPE_YES" ] ; then
                echo $UT_TYPE_ANYTHING_SERVER_PLUGINS "is making"
		qmake $TESTS_FOLDER/deepin-anything-server-plugins/test-deepin-anything-server-plugins.pro $QMAKE_ARGS
		make -j$CPU_NUMBER
        fi

	deepin_anything_server_plugins_extract_path="*/src/deepin-anything-server-plugins/*"
	deepin_anything_server_plugins_remove_path="*/third-party/* *tests* */dde-file-manager-lib/vault/openssl/* */build-ut/* */dde-file-manager-lib/vault/qrencode/* */dde-desktop/dbus/* */dde-wallpaper-chooser/dbus/* *moc_* *qrc_*"
	# report的文件夹，报告后缀名，编译路径，可执行程序名，正向解析设置，逆向解析设置
	./../../../tests/ut-target-running.sh $BUILD_DIR dde-anythingmonitor $DIR_TEST_DEEPIN_ANYTHING_SERVER_PLUGINS test-dde-anythingmonitor  "$deepin_anything_server_plugins_extract_path"  "$deepin_anything_server_plugins_remove_path" $SHOW_REPORT
        check_ut_result $? $UT_TYPE_ANYTHING_SERVER_PLUGINS
fi

echo "end dde-file-manager all UT cases"

exit 0
