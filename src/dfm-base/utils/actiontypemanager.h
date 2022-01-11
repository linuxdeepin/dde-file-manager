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
#ifndef ACTIONTYPEMANAGER_H
#define ACTIONTYPEMANAGER_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/dfm_actiontype_defines.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/widgets/action/actiondatacontainer.h"

#include <QObject>

DFMBASE_BEGIN_NAMESPACE

class ActionDataContainer;
class ActionTypeManagerPrivate;
class ActionTypeManager : public QObject
{
    Q_OBJECT
public:
    static ActionTypeManager &instance();
    QPair<int, ActionDataContainer> registerActionType(const QString &actText);
    ActionDataContainer actionNameByType(const int actType,
                                         ActionDataContainer defaultAct = ActionDataContainer());
    QVector<ActionDataContainer> actionNameListByTypes(const QVector<ActionType> &actTypes);
    void recycleActionType(int actType);
    void actionGlobleEventBind(ActionType actType, GlobalEventType eventType);
    void actionGlobleEventUnBind(ActionType actType);
    GlobalEventType actionGlobalEvent(ActionType actType);

private:
    explicit ActionTypeManager(QObject *parent = nullptr);
    ~ActionTypeManager();
    void initDefaultActionData();
    void initDefaultActionEvent();

private:
    QScopedPointer<ActionTypeManagerPrivate> d;
};

DFMBASE_END_NAMESPACE
#endif   // ACTIONTYPEMANAGER_H
