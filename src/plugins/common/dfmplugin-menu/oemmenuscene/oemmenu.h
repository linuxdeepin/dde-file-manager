// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OEMMENU_H
#define OEMMENU_H

#include "dfmplugin_menu_global.h"

#include <QObject>
#include <QAction>
#include <QScopedPointer>

namespace dfmplugin_menu {

class OemMenuPrivate;
class OemMenu : public QObject
{
    Q_OBJECT
public:
    explicit OemMenu(QObject *parent = nullptr);
    ~OemMenu();

    void loadDesktopFile();
    QList<QAction *> emptyActions(const QUrl &currentDir, bool onDesktop = false);
    QList<QAction *> normalActions(const QList<QUrl> &files, bool onDesktop = false);
    QList<QAction *> focusNormalActions(const QUrl &foucs, const QList<QUrl> &files, bool onDesktop = false);
    QPair<QString, QStringList> makeCommand(const QAction *action, const QUrl &dir, const QUrl &focus, const QList<QUrl> &files);

private:
    QScopedPointer<OemMenuPrivate> d;

private:
    Q_DISABLE_COPY(OemMenu)
};

}

#endif   // OEMMENU_H
