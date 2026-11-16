// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWUTILS_H
#define WINDOWUTILS_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>

QT_BEGIN_NAMESPACE
class QScreen;
class QString;
class QWidget;
QT_END_NAMESPACE

namespace dfmbase {

class WindowUtils
{
public:
    static bool isX11();

    static bool isWayLand();

    static bool keyShiftIsPressed();

    static bool keyCtrlIsPressed();

    static bool keyAltIsPressed();

    static QScreen *cursorScreen();

    static void closeAllFileManagerWindows();

    /*!
     * \brief Exports the xdg-activation token passed in by the caller
     *
     * Qt reads that token from the environment when a surface is shown or
     * activation is requested, which is the only way a Wayland client can ask
     * for the focus to follow a launch.
     */
    static void setActivationToken(const QString &token);

    /*!
     * \brief Brings \a widget to the front and gives it the keyboard focus
     *
     * Restores the window first when the platform allows it. On Wayland a
     * client cannot lift its own minimized state, so that part is delegated to
     * the compositor and the result falls back to a plain activation request.
     */
    static void activateWindow(QWidget *widget);
};

}   // namespace dfmbase
#endif   // WINDOWUTILS_H
