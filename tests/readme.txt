#运行UT类型 all dde_file_manager dde-file-manager-lib dde-desktop dde-dock-plugins dde-file-manager-plugins dde-file-thumbnail-tool deepin-anything-server-plugins
1. 默认为支持所有ut case, cpp-check运行
./test-prj-running.sh
2. 支持重编译并运行特殊UT, 比如 dde-desktop
./test-prj-running.sh --ut dde-desktop

3. 支持不再编译，只跑某个特殊UT, 比如 dde-desktop
./test-prj-running.sh --clear no --ut dde-desktop
