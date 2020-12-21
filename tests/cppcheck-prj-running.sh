#!/bin/bash

# 定位脚本所在父目录
PROJECT_FOLDER=${1}
echo $PROJECT_FOLDER

# 定位脚本所在父目录下的某文件
BUILD_DIR=${2}

# 打印当前目录，当前目录应当是 build-ut
echo `pwd`
echo "start dde-file-manager cppcheck case"

# 下面是cppcheck检测

# 过滤掉第三方库以及单元测试
FILTER_SYMBOL=" -i "
DDE_DESKTOP_FILTER_PATH=$FILTER_SYMBOL$PROJECT_FOLDER/dde-desktop/tests/
DDE_FILE_MANAGER_FILTER_PATH=$FILTER_SYMBOL$PROJECT_FOLDER/dde-file-manager/tests/
DDE_FILE_MANAGER_LIB_FILTER_PATH=$FILTER_SYMBOL$PROJECT_FOLDER/dde-file-manager-lib/3rdParty/$FILTER_SYMBOL$PROJECT_FOLDER/dde-file-manager-lib/search/fsearch/$FILTER_SYMBOL$PROJECT_FOLDER/dde-file-manager-lib/vault/openssl/$FILTER_SYMBOL$PROJECT_FOLDER/dde-file-manager-lib/vault/qrencode/
DDE_FILE_MANAGER_DAEMON_FILTER_PATH=$FILTER_SYMBOL$PROJECT_FOLDER/dde-file-manager-daemon/tests/
DDE_FILE_MANAGER_PLUGINS_FILTER_PATH=$FILTER_SYMBOL$PROJECT_FOLDER/dde-file-manager-plugins/pluginPreview/dde-image-preview-plugin/tests$FILTER_SYMBOL$PROJECT_FOLDER/dde-file-manager-plugins/pluginPreview/dde-music-preview-plugin/tests$FILTER_SYMBOL$PROJECT_FOLDER/dde-file-manager-plugins/pluginPreview/dde-pdf-preview-plugin/tests$FILTER_SYMBOL$PROJECT_FOLDER/dde-file-manager-plugins/pluginPreview/dde-text-preview-plugin/tests$FILTER_SYMBOL$PROJECT_FOLDER/dde-file-manager-plugins/pluginPreview/dde-video-preview-plugin/tests$FILTER_SYMBOL$PROJECT_FOLDER/dde-file-manager-plugins/pluginView/tests
DDE_DOCK_PLUGINS_FILTER_PATH=$FILTER_SYMBOL$PROJECT_FOLDER/dde-dock-plugins/disk-mount/tests/
DDE_FILE_THUMBNAIL_TOOL_FILTER_PATH=$FILTER_SYMBOL$PROJECT_FOLDER/dde-file-thumbnail-tool/video/tests/
DEEPIN_ANYTHING_SERVER_PLUGINS_FILTER_PATH=$FILTER_SYMBOL$PROJECT_FOLDER/deepin-anything-server-plugins/dde-anythingmonitor/tests/
OTHER_FILTER_PATH=$FILTER_SYMBOL$BUILD_DIR$FILTER_SYMBOL$PROJECT_FOLDER/third-party/

# report目录
REPORT_FILE_PATH=$BUILD_DIR/cppcheck_dde-file-manager.xml
# 打印cppcheck命令
echo "cppcheck --enable=all --output-file="$REPORT_FILE_PATH "--xml" $DDE_DESKTOP_FILTER_PATH $DDE_FILE_MANAGER_FILTER_PATH $DDE_FILE_MANAGER_LIB_FILTER_PATH $DDE_FILE_MANAGER_DAEMON_FILTER_PATH $DDE_FILE_MANAGER_PLUGINS $DDE_DOCK_PLUGINS_FILTER_PATH $DDE_FILE_THUMBNAIL_TOOL_FILTER_PATH $DEEPIN_ANYTHING_SERVER_PLUGINS $OTHER_FILTER_PATH $PROJECT_FOLDER
# 执行cppcheck命令
cppcheck --enable=all --output-file=$REPORT_FILE_PATH --xml $DDE_DESKTOP_FILTER_PATH $DDE_FILE_MANAGER_FILTER_PATH $DDE_FILE_MANAGER_LIB_FILTER_PATH $DDE_FILE_MANAGER_DAEMON_FILTER_PATH $DDE_FILE_MANAGER_PLUGINS $DDE_DOCK_PLUGINS_FILTER_PATH $DDE_FILE_THUMBNAIL_TOOL_FILTER_PATH $DEEPIN_ANYTHING_SERVER_PLUGINS $OTHER_FILTER_PATH $PROJECT_FOLDER

echo "end dde-file-manager cppcheck case"

exit 0
