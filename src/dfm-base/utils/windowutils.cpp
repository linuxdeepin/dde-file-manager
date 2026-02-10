// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "windowutils.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QApplication>
#include <QScreen>

bool DFMBASE_NAMESPACE::WindowUtils::isX11()
{
    if (QGuiApplication::platformName() == "xcb") {
        return true;
    }

    // 检查DISPLAY环境变量
    const char *display = std::getenv("DISPLAY");
    if (!display || display[0] == '\0') {
        return false;
    }

    // 检查XDG_SESSION_TYPE环境变量
    const char *session_type = std::getenv("XDG_SESSION_TYPE");
    if (session_type && strcmp(session_type, "x11") == 0) {
        return true;
    }

    // 如果DISPLAY存在但XDG_SESSION_TYPE不是wayland，可能是X11
    if (session_type && strcmp(session_type, "wayland") != 0) {
        return true;
    }

    return false;
}

bool DFMBASE_NAMESPACE::WindowUtils::isWayLand()
{
    if (QGuiApplication::platformName() == "wayland") {
        return true;
    }

    // 方法1：检查WAYLAND_DISPLAY环境变量
    const char *wayland_display = std::getenv("WAYLAND_DISPLAY");
    if (wayland_display && wayland_display[0] != '\0') {
        return true;
    }

    // 方法2：检查XDG_SESSION_TYPE环境变量
    const char *session_type = std::getenv("XDG_SESSION_TYPE");
    if (session_type && strcmp(session_type, "wayland") == 0) {
        return true;
    }

    return false;
}

bool DFMBASE_NAMESPACE::WindowUtils::keyShiftIsPressed()
{
    return qApp->keyboardModifiers() == Qt::ShiftModifier;
}

bool DFMBASE_NAMESPACE::WindowUtils::keyCtrlIsPressed()
{
    return qApp->queryKeyboardModifiers() == Qt::ControlModifier;
}

bool DFMBASE_NAMESPACE::WindowUtils::keyAltIsPressed()
{
    return qApp->keyboardModifiers() == Qt::AltModifier;
}

QScreen *DFMBASE_NAMESPACE::WindowUtils::cursorScreen()
{
    QScreen *cursorScreen = nullptr;
    const QPoint &cursorPos = QCursor::pos();

    QList<QScreen *> screens = qApp->screens();
    QList<QScreen *>::const_iterator it = screens.begin();
    for (; it != screens.end(); ++it) {
        if ((*it)->geometry().contains(cursorPos)) {
            cursorScreen = *it;
            break;
        }
    }

    if (!cursorScreen)
        cursorScreen = qApp->primaryScreen();

    return cursorScreen;
}

void DFMBASE_NAMESPACE::WindowUtils::closeAllFileManagerWindows()
{
    auto winIds { FileManagerWindowsManager::instance().windowIdList() };

    for (auto id : winIds) {
        FileManagerWindow *window { FileManagerWindowsManager::instance().findWindowById(id) };
        if (window)
            window->close();
    }

    winIds = FileManagerWindowsManager::instance().windowIdList();
    if (winIds.count() > 0)
        qApp->quit();
}
