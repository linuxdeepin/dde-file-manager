/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#ifndef ACTIONDATACONTAINER_H
#define ACTIONDATACONTAINER_H
#include "dfm-base/dfm_base_global.h"

#include <QString>
#include <QIcon>
#include <QAction>

DFMBASE_BEGIN_NAMESPACE

using TriggeredFunc = std::function<void(QAction *)>;
using DestroyedFunc = std::function<void(QAction *)>;

class ActionDataContainer
{
public:
    ActionDataContainer();
    ActionDataContainer(int type, const QString &name);
    ActionDataContainer(const ActionDataContainer &other);
    int actionType();
    QString name() const;
    void setName(const QString &name);
    QIcon icon() const;
    void setIcon(const QIcon &icon);
    QVariant data();
    void setData(const QVariant &var);
    TriggeredFunc triggeredFunc();
    void registerTriggeredFunc(TriggeredFunc func);
    DestroyedFunc destroyedFunc();
    void registerdestroyedFunc(DestroyedFunc func);

private:
    int actType { -1 };
    QString actionName;
    QIcon actionIcon;
    QString actionCommand;
    QVariant actionData;
    TriggeredFunc actionTriggeredFunc;
    DestroyedFunc actionDestroyedFunc;
    //    HoveredFunc hoveredFunc; // todo
};

DFMBASE_END_NAMESPACE

#endif   // ACTIONDATACONTAINER_H
