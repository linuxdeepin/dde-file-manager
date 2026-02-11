// SPDX-FileCopyrightText: 2023 - 2026 ~ 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mymenuplugin.h"

#include <sys/types.h>
#include <sys/xattr.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cassert>
#include <iostream>

#include <dfm-extension/menu/dfmextmenu.h>
#include <dfm-extension/menu/dfmextmenuproxy.h>
#include <dfm-extension/menu/dfmextaction.h>

namespace Exapmle {
USING_DFMEXT_NAMESPACE

MyMenuPlugin::MyMenuPlugin()
    : DFMEXT::DFMExtMenuPlugin()
{
    registerInitialize([this](DFMEXT::DFMExtMenuProxy *proxy) {
        initialize(proxy);
    });
    registerBuildNormalMenu([this](DFMExtMenu *main, const std::string &currentPath,
                                   const std::string &focusPath, const std::list<std::string> &pathList,
                                   bool onDesktop) {
        return buildNormalMenu(main, currentPath, focusPath, pathList, onDesktop);
    });
    registerBuildEmptyAreaMenu([this](DFMExtMenu *main, const std::string &currentPath, bool onDesktop) {
        return buildEmptyAreaMenu(main, currentPath, onDesktop);
    });
}

MyMenuPlugin::~MyMenuPlugin()
{
}

void MyMenuPlugin::initialize(DFMExtMenuProxy *proxy)
{
    m_proxy = proxy;
}

bool MyMenuPlugin::buildNormalMenu(DFMExtMenu *main, const std::string &currentPath,
                                   const std::string &focusPath, const std::list<std::string> &pathList,
                                   bool onDesktop)
{
    // 可使用此方法管理自己开辟的内存，菜单关闭时被调用
    // 但是并不需要使用代理 m_proxy->deleteMenu / delteAction
    // 因为只要创建的 menu 和 action 被加入了菜单 main，那么
    // 它们的内存会被自动释放
    auto memTest { new int };
    main->registerDeleted([memTest](DFMExtMenu *self) {
        delete memTest;
    });

    (void)onDesktop;
    (void)currentPath;
    (void)focusPath;

    // 通过代理创建 action
    auto rootAction { m_proxy->createAction() };
    rootAction->setText("角标管理");

    // 通过代理创建 menu
    auto menu { m_proxy->createMenu() };

    // 二级菜单在 Hover 中创建，以减少一级菜单显示的性能开销
    rootAction->setMenu(menu);
    rootAction->registerHovered([this, pathList](DFMExtAction *action) {
        if (!action->menu()->actions().empty())
            return;
        auto favoriteEmblemAct { m_proxy->createAction() };
        favoriteEmblemAct->setText("角标设置为favorite");
        favoriteEmblemAct->setIcon("emblem-favorite");
        favoriteEmblemAct->registerTriggered([this, pathList](DFMExtAction *, bool) {
            std::for_each(pathList.begin(), pathList.end(), [this](const std::string &path) {
                setEmblemIcon(path, "emblem-favorite");
            });
        });

        auto defaultEmblemAct { m_proxy->createAction() };
        defaultEmblemAct->setIcon("emblem-default");
        defaultEmblemAct->setText("角标设置为default");
        defaultEmblemAct->registerTriggered([this, pathList](DFMExtAction *, bool) {
            std::for_each(pathList.begin(), pathList.end(), [this](const std::string &path) {
                setEmblemIcon(path, "emblem-default");
            });
        });

        auto clearEmbelmAct { m_proxy->createAction() };
        clearEmbelmAct->setIcon("emblem-important");
        clearEmbelmAct->setText("清除角标");
        clearEmbelmAct->registerTriggered([this, pathList](DFMExtAction *, bool) {
            std::for_each(pathList.begin(), pathList.end(), [this](const std::string &path) {
                clearEmblemIcon(path);
            });
        });

        action->menu()->addAction(favoriteEmblemAct);
        action->menu()->addAction(defaultEmblemAct);
        action->menu()->addAction(clearEmbelmAct);
    });

    main->addAction(rootAction);
    return true;
}

bool MyMenuPlugin::buildEmptyAreaMenu(DFMExtMenu *main, const std::string &currentPath, bool onDesktop)
{
    assert(main);

    // 通过代理创建 action，此 action 在堆区分配，不自行释放将内存泄露！
    auto action { m_proxy->createAction() };

    // 通过 onDesktop 区分业务在桌面还是文管
    if (onDesktop)
        action->setText("从文件管理器打开桌面");
    else
        action->setText("打开当前路径");
    // 添加图标，也支持图片的文件绝对路径
    // 例如：/usr/share/icons/Adwaita/16x16/emblems/emblem-generic.png
    action->setIcon("emblem-generic");

    // action 被点击触发的业务处理
    action->registerTriggered([currentPath](DFMExtAction *self, bool checked) {
        (void)self;
        (void)checked;
        pid_t pid = fork();
        if (pid == 0) {
            // 子进程中调用 execvp 函数来执行 dde-file-manager 进程，并传递参数
            char *argv[] { "/usr/bin/dde-file-manager", "-n", const_cast<char *>(currentPath.c_str()), NULL };
            execvp(argv[0], argv);
        } else if (pid > 0) {
            // 父进程等待子进程结束
            int status;
            waitpid(-1, &status, WNOHANG);
        } else {
            perror("fork failed");
        }
    });

    // TODO: add interface id()
    auto actions = main->actions();
    auto it = std::find_if(actions.cbegin(), actions.cend(), [](const DFMExtAction *action) {
        const std::string &text = action->text();
        return (text.find("刷新") == 0) || (text.find("Refresh") == 0);
    });

    if (it != actions.cend()) {
        auto separator = m_proxy->createAction();
        separator->setSeparator(true);
        main->insertAction(*it, separator);
        main->insertAction(*it, action);
    } else {
        main->addAction(action);
    }

    return true;
}

void MyMenuPlugin::setEmblemIcon(const std::string &filePath, const std::string &iconName)
{
    std::cout << "set emblem icon " << iconName << " for " << filePath << std::endl;
    const std::string path { removeScheme(filePath) };
    int result = setxattr(path.c_str(), "user.icon", iconName.c_str(), iconName.size(), 0);
    if (result == -1)
        perror("setxattr");
}

void MyMenuPlugin::clearEmblemIcon(const std::string &filePath)
{
    std::cout << "clear emblem icon for " << filePath;
    const std::string path { removeScheme(filePath) };
    int result = removexattr(path.c_str(), "user.icon");
    if (result == -1)
        perror("removexattr");
}

// 在 V5 版本的文管中，menu 接收到的是文件 url 的字符串，而 V6 则直接是文件路径
std::string MyMenuPlugin::removeScheme(const std::string &url)
{
    std::string result = url;

    // 查找第一个冒号后的斜杠位置
    size_t startPos = result.find("://");
    if (startPos != std::string::npos) {
        startPos = result.find('/', startPos + 3);   // 跳过冒号和两个斜杠
        if (startPos != std::string::npos) {
            result = result.substr(startPos);
        }
    }

    return result;
}

}   // namespace Exapmle
