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
#ifndef OEMMENU_P_H
#define OEMMENU_P_H

#include "dfmplugin_menu_global.h"

#include <DDesktopEntry>

#include <QObject>
#include <QTimer>
#include <QAction>
#include <QSharedPointer>
#include <QSharedData>

DPMENU_BEGIN_NAMESPACE

class OemMenu;
class OemMenuPrivate : public QSharedData
{
public:
    explicit OemMenuPrivate(OemMenu *qq);
    ~OemMenuPrivate();

    QStringList getValues(const Dtk::Core::DDesktopEntry &entry, const QString &key, const QString &aliasKey, const QString &section = "Desktop Entry", const QStringList &whiteList = {}) const;

    bool isMimeTypeSupport(const QString &mt, const QStringList &fileMimeTypes) const;
    bool isMimeTypeMatch(const QStringList &fileMimeTypes, const QStringList &supportMimeTypes) const;
    bool isActionShouldShow(const QAction *action, bool onDesktop) const;
    bool isSchemeSupport(const QAction *action, const QUrl &url) const;
    bool isSuffixSupport(const QAction *action, const QUrl &url, const bool allEx7z = false) const;
    bool isAllEx7zFile(const QList<QUrl> &files) const;
    bool isValid(const QAction *action, const QUrl &url, const bool onDesktop, const bool allEx7z = false) const;

    void clearSubMenus();
    void setActionProperty(QAction *const action, const Dtk::Core::DDesktopEntry &entry, const QString &key, const QString &section = "Desktop Entry") const;
    void setActionData(QList<QAction *> actions, const QStringList &files) const;
    void setActionData(QAction *action, const QStringList &files) const;
    void appendParentMineType(const QStringList &parentmimeTypes, QStringList &mimeTypes) const;

    QSharedPointer<QTimer> delayedLoadFileTimer;
    QSharedPointer<QObject> menuActionHolder;
    QMap<QString, QList<QAction *>> actionListByType;
    QList<QMenu *> subMenus;

    QStringList oemMenuPath;
    QStringList menuTypes;
    QStringList actionProperties;

    OemMenu *q;
};

DPMENU_END_NAMESPACE

#endif   // OEMMENU_P_H
