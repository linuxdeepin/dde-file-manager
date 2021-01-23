### 命令参数说明

运行UT类型 no all dde_file_manager dde-file-manager-lib dde-desktop dde-dock-plugins dde-file-manager-plugins dde-file-thumbnail-tool deepin-anything-server-plugins

--clear:默认情况为yes删除当前build-ut下所有数据， 使用 --clear no 不进行清除操作
--ut:指定ut项目类型，可以使用类型：no all dde_file_manager dde-file-manager-lib dde-desktop dde-dock-plugins dde-file-manager-plugins dde-file-thumbnail-tool deepin-anything-server-plugins
--rebuild：默认为yes进行重新编译， 使用--rebuild no 不进行重新编译
--cppcheck：默认为no 不进行项目cpp-check文件扫描， 使用--cppcheck yes 进行cpp-check扫描
--show：默认为no 不显示报表， 使用--show yes 显示当前报表
--cpu：当前使用CPU数目，默认为16

### 使用用例

1. 默认为支持所有ut case, cppcheck运行
./test-prj-running.sh

2. 支持重编译并运行特殊UT, 比如 dde-desktop
./test-prj-running.sh --ut dde-desktop

3. 支持不再编译，只跑某个特殊UT, 比如 dde-desktop
./test-prj-running.sh --clear no --ut dde-desktop

4. 支持只单独运行ut或者cppcheck
只运行ut
./test-prj-running.sh --cppcheck no
只运行cppcheck
./test-prj-running.sh --ut no 或者 ./test-prj-running.sh --ut no --cppcheck yes

5. 直接运行dde_file_manager UT case，并查看case 与 覆盖率情况
    ./test-prj-running.sh --clear no --ut dde-file-manager --rebuild no

