/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef DESKTOPFILEINFO_H
#define DESKTOPFILEINFO_H

#include "dfileinfo.h"

#include <QObject>

#define DESKTOP_SURRIX "desktop"

class DesktopFileInfoPrivate;
class DesktopFileInfo : public DFileInfo
{
public:
    DesktopFileInfo(const DUrl &fileUrl);
    DesktopFileInfo(const QFileInfo &fileInfo);

    ~DesktopFileInfo();

    QString getName() const;
    QString getExec() const;
    QString getIconName() const;
    QString getType() const;
    QStringList getCategories() const;

    QIcon fileIcon() const Q_DECL_OVERRIDE;
    QString fileDisplayName() const Q_DECL_OVERRIDE;
    QString fileNameOfRename() const override;
    QString baseNameOfRename() const override;
    QString suffixOfRename() const override;

    void refresh() Q_DECL_OVERRIDE;

    QString iconName() const Q_DECL_OVERRIDE;
    QString genericIconName() const Q_DECL_OVERRIDE;

    static QMap<QString, QVariant> getDesktopFileInfo(const DUrl &fileUrl);

    QVector<MenuAction> menuActionList(MenuType type = SingleFile) const Q_DECL_OVERRIDE;
    QSet<MenuAction> disableMenuActionList() const Q_DECL_OVERRIDE;
    QList<QIcon> additionalIcon() const;
    Qt::DropActions supportedDragActions() const Q_DECL_OVERRIDE;

    bool canDrop() const Q_DECL_OVERRIDE;
    bool canTag() const Q_DECL_OVERRIDE;

    static DUrl trashDesktopFileUrl();
    static DUrl computerDesktopFileUrl();
    static DUrl homeDesktopFileUrl();

private:
    Q_DECLARE_PRIVATE(DesktopFileInfo)
};

#endif // DESKTOPFILEINFO_H
