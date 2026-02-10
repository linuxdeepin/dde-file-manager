// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "myfileplugin.h"

#include <sys/types.h>
#include <sys/xattr.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cassert>
#include <sstream>

#include <dfm-extension/file/dfmextfileoperationhook.h>

namespace Exapmle {
USING_DFMEXT_NAMESPACE
/*!
 * \brief 文件打开操作的钩子函数实现。当用户在文件管理器/桌面中打开文件时,
 * 可以通过此钩子函数拦截并实现自定义的打开方式。
 * \param srcPaths 需要打开的文件路径列表
 * \param ignorePaths 不处理的文件路径列表,这些文件将使用默认行为打开
 * \return 返回值说明:
 *         - true: 使用此钩子函数处理文件打开操作,替代文件管理器默认行为
 *         - false: 不处理文件打开操作,使用文件管理器默认行为
 *
 * 此函数实现以下业务逻辑:
 * 1. 检查系统中是否安装了kate编辑器
 * 2. 遍历所有传入的文件路径:
 *    - 对于.xml后缀的文件,收集起来准备用kate打开
 *    - 对于非.xml文件,加入到ignorePaths列表中,使用默认程序打开
 * 3. 如果找到xml文件:
 *    - 使用fork/exec启动kate打开所有xml文件
 *    - 返回true表示已处理xml文件
 * 4. 如果没有xml文件:
 *    - 返回false让文件管理器使用默认行为
 *
 * 这个示例展示了如何:
 * - 实现文件类型过滤
 * - 使用外部程序打开特定类型文件
 * - 让其他类型文件保持默认行为
 */
static bool openFiles(const std::vector<std::string> &srcPaths, std::vector<std::string> *ignorePaths)
{
    assert(ignorePaths);

    // 检查 kate 是否存在, 需要自行安装 kate
    if (access("/usr/bin/kate", X_OK) != 0) {
        return false;
    }

    // 收集所有xml文件
    std::string xmlFiles;
    bool hasXml = false;

    for (const auto &path : srcPaths) {
        if (path.length() > 4 && path.substr(path.length() - 4) == ".xml") {
            if (hasXml) {
                xmlFiles += " ";
            }
            xmlFiles += path;
            hasXml = true;
        } else {
            // 非xml文件加入到ignorePaths
            ignorePaths->push_back(path);
        }
    }

    // 如果没有xml文件则返回false
    if (!hasXml) {
        return false;
    }

    // 使用fork和execvp启动kate
    pid_t pid = fork();
    if (pid == 0) {
        // 子进程
        // 将文件字符串按空格分割成数组
        std::vector<std::string> files;
        std::istringstream iss(xmlFiles);
        std::string file;
        while (iss >> file) {
            files.push_back(file);
        }

        // 创建参数数组：程序名 + 所有文件 + nullptr
        std::vector<char*> argv(files.size() + 2);
        argv[0] = const_cast<char*>("/usr/bin/kate");
        for (size_t i = 0; i < files.size(); ++i) {
            argv[i + 1] = const_cast<char*>(files[i].c_str());
        }
        argv[files.size() + 1] = nullptr;

        execvp(argv[0], argv.data());
        exit(1);  // 如果execvp失败则退出
    } else if (pid > 0) {
        // 父进程
        int status;
        waitpid(-1, &status, WNOHANG);
        return true;
    }

    return false;
}

MyFilePlugin::MyFilePlugin()
    : DFMExtFilePlugin()
{
    registerFileOperationHooks([this](DFMExtFileOperationHook *hook) {
        fileOperationHooks(hook);
    });
}

void MyFilePlugin::fileOperationHooks(DFMExtFileOperationHook *hook)
{
    if (!hook)
        return;

    hook->registerOpenFiles(openFiles);
}

}   // namespace Exapmle
