/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#ifndef OPERATORREVOCATION_H
#define OPERATORREVOCATION_H

#include "dfmabstracteventhandler.h"
#include "dfmevent.h"

#include <QStack>
class RevocationMgrInterface;

DFM_BEGIN_NAMESPACE

class OperatorRevocation : public QObject, public DFMAbstractEventHandler
{
    Q_OBJECT
public:
    /**
     * @brief The RevocationEventType enum
     *  撤销事件类型
     */
    enum RevocationEventType
    {
        DFM_NO_EVENT = -1,
        DFM_FILE_MGR_EVENT = 0,
        DFM_DESKTOP_EVENT = 1
    };

    static OperatorRevocation *instance();

protected:
    bool fmEvent(const QSharedPointer<DFMEvent> &event, QVariant *resultData = nullptr) override;

    OperatorRevocation();

protected slots:
    void slotRevocationEvent();

private:
    bool revocation();
    QString getProcessName();

    QStack<DFMEvent> operatorStack;

    RevocationMgrInterface *m_dbusInterface = nullptr;

    RevocationEventType m_eventType = DFM_NO_EVENT;
};

DFM_END_NAMESPACE

#endif // OPERATORREVOCATION_H
