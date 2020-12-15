/*
 * Copyright (C) 2020 Deepin Technology Co., Ltd.
 *
 * Author:     Lu Zhen <luzhen@uniontech.com>
 *
 * Maintainer: Lu Zhen <luzhen@uniontech.com>
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


#ifndef REVOCATIONMANAGER_H
#define REVOCATIONMANAGER_H

#include <QObject>
#include <QStack>
#include <QDBusContext>

class RevocationMgrAdaptor;
/**
 * @brief The RevocationManager class
 */
class RevocationManager : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    enum RevocationEventType
    {
        DFM_NO_EVENT = -1,
        DFM_FILE_MGR = 0,
        DFM_DESKTOP = 1
    };

    explicit RevocationManager(QObject *parent = nullptr);
    ~RevocationManager();

    static QString ObjectPath;

public slots:
    /**
     * @brief pushEvent 入栈事件
     * @param event 0:filemgr 1:desktop
     */
    void pushEvent(int event);

    /**
     * @brief popEvent 弹出最新事件
     * @returns 事件 0:filemgr 1:desktop
     */
    int popEvent();

signals:
    void fmgrRevocationAction();
    void deskRevocationAction();

private:
    RevocationMgrAdaptor* m_adaptor = nullptr;
    QStack<RevocationEventType> m_eventStack;
};

#endif // REVOCATIONMANAGER_H
