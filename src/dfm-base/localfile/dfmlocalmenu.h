/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef DFMLOCALMENU_H
#define DFMLOCALMENU_H

#include "dfm-base/base/dfmglobal.h"

#include <QObject>

#include <QMenu>


typedef DFMGlobal::MenuAction MenuAction;
class DFMLocalMenuPrivate;
class DFMLocalMenu : public QObject
{
    Q_OBJECT
    enum MenuType {
        SingleFile,
        MultiFiles,
        MultiFilesSystemPathIncluded,
        SpaceArea
    };

    Q_DECLARE_PRIVATE(DFMLocalMenu)
    QSharedPointer<DFMLocalMenuPrivate> d_ptr;
public:
    explicit DFMLocalMenu(const QString &filePath);

    virtual QVector<MenuAction> menuActionList(MenuType type) const;
    virtual QMap<MenuAction, QVector<MenuAction>>subMenuActionList(MenuType type = SingleFile) const;
    virtual QSet<MenuAction> disableMenuActionList() const;
    virtual MenuAction menuActionByColumnRole(int role) const;
    virtual QList<int> sortSubMenuActionUserColumnRoles() const;

    bool isAddOemExternalAction();
    void setAddOemExternalAction(bool isAdd);
    void setIsNeedLoadCustomActions(bool needCustom);
    bool isNeedLoadCustomActions();

};

#endif // DFMLocalMenu_H
