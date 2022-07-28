/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
    static OemMenu *instance();
    ~OemMenu();

    void loadDesktopFile();
    QList<QAction *> emptyActions(const QUrl &currentDir, bool onDesktop = false);
    QList<QAction *> normalActions(const QList<QUrl> &files, bool onDesktop = false);
    QPair<QString, QStringList> makeCommand(const QAction *action, const QUrl &dir, const QUrl &foucs, const QList<QUrl> &files);

protected:
    explicit OemMenu(QObject *parent = nullptr);

private:
    QScopedPointer<OemMenuPrivate> d;

private:
    Q_DISABLE_COPY(OemMenu)
};

}

#endif   // OEMMENU_H
