// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "windowutils.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QApplication>
#include <QScreen>

bool DFMBASE_NAMESPACE::WindowUtils::isWayLand()
{
    //! This function can only be called after QApplication to return a valid value, before it will return a null value
    Q_ASSERT(qApp);
    return QApplication::platformName() == "wayland";
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
