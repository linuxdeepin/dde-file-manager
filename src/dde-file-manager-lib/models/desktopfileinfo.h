/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
    explicit DesktopFileInfo(const DUrl &fileUrl);
    explicit DesktopFileInfo(const QFileInfo &fileInfo);

    ~DesktopFileInfo() override;

    QString getName() const;
    QString getExec() const;
    QString getIconName() const;
    QString getType() const;
    QStringList getCategories() const;

    QIcon fileIcon() const override;
    QString fileDisplayName() const override;
    QString fileNameOfRename() const override;
    QString baseNameOfRename() const override;
    QString suffixOfRename() const override;

    void refresh(const bool isForce = false) override;

    QString iconName() const override;
    QString genericIconName() const override;

    static QMap<QString, QVariant> getDesktopFileInfo(const DUrl &fileUrl);

    QVector<MenuAction> menuActionList(MenuType type = SingleFile) const override;
    QSet<MenuAction> disableMenuActionList() const override;
    QList<QIcon> additionalIcon() const;
    Qt::DropActions supportedDragActions() const override;

    bool canDrop() const override;
    bool canTag() const override;
    bool canMoveOrCopy() const override;

    static DUrl trashDesktopFileUrl();
    static DUrl computerDesktopFileUrl();
    static DUrl homeDesktopFileUrl();

private:
    Q_DECLARE_PRIVATE(DesktopFileInfo)
};

#endif // DESKTOPFILEINFO_H
