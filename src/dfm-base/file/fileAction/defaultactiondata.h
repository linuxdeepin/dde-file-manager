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
#ifndef DEFAULTACTIONDATA_H
#define DEFAULTACTIONDATA_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/dfm_actiontype_defines.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/widgets/action/actiondatacontainer.h"

#include <QObject>

DFMBASE_BEGIN_NAMESPACE

class DefaultActionDataPrivate;
class DefaultActionData : public QObject
{
    Q_OBJECT
public:
    explicit DefaultActionData(QObject *parent = nullptr);
    ~DefaultActionData();
    ActionDataContainer actionDataContainerByType(const int actType,
                                                  const ActionDataContainer &defaultAct = ActionDataContainer());
    bool addSubActionType(ActionType parentType, const ActionDataContainer &actionData);
    bool contains(ActionType type);

private:
    void initDefaultActionData();
    void initDefaultActionEvent();
    ActionDataContainer addDefaultActionType(ActionType type, const QString &actPredicate, const QString &actionText);

private:
    QScopedPointer<DefaultActionDataPrivate> d;
};

DFMBASE_END_NAMESPACE
#endif   // DEFAULTACTIONDATA_H
