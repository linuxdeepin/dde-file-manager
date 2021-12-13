/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef ExtMenuPRIVATEIMPL_H
#define ExtMenuPRIVATEIMPL_H

#include <menu/private/dfmextmenuprivate.h>

#include <QObject>

class QAction;
class QMenu;
class DFMExtMenuImpl;
class DFMExtMenuImplPrivate : public QObject, public DFMEXT::DFMExtMenuPrivate
{
    Q_OBJECT
public:
    DFMExtMenuImplPrivate(DFMExtMenuImpl *qImpl, QMenu *m = nullptr);
    virtual ~DFMExtMenuImplPrivate() override;

    DFMExtMenuImpl *menuImpl() const;
    QMenu *qmenu() const;
    bool isInterior() const;

    std::string title() const Q_DECL_OVERRIDE;
    void setTitle(const std::string &title) Q_DECL_OVERRIDE;

    std::string icon() const Q_DECL_OVERRIDE;
    void setIcon(const std::string &iconName) Q_DECL_OVERRIDE;

    bool addAction(DFMEXT::DFMExtAction *action) Q_DECL_OVERRIDE;
    bool insertAction(DFMEXT::DFMExtAction *before, DFMEXT::DFMExtAction *action) Q_DECL_OVERRIDE;

    DFMEXT::DFMExtAction *menuAction() const Q_DECL_OVERRIDE;
    std::list<DFMEXT::DFMExtAction *> actions() const Q_DECL_OVERRIDE;
private slots:
    void onActionHovered(QAction *action);
    void onActionTriggered(QAction *action);
    void onMenuDestoryed(QObject *obj);
    void deleteParent();
protected:
    QMenu *menu = nullptr;
    DFMExtMenuImpl *q = nullptr;
    bool interiorEntity = true; //是否是内部创建
};

#endif // ExtMenuPRIVATEIMPL_H
