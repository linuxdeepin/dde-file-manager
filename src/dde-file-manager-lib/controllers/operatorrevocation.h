/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#include <QMutex>
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
    /**
    * @brief slotRevocationEvent 根据用户名文管进行对应的撤销动作
    */
    void slotRevocationEvent(const QString& user);

private:
    /**
     * @brief initialize 建立dbus连接，获取程序类型
     * @return
     */
    bool initialize();

    /**
     * @brief revocation 执行恢复操作
     * @return
     */
    bool revocation();

    /**
     * @brief getProcessName 获取当前进程名
     * @return
     */
    QString getProcessName();

    /**
     * @brief pushEvent 存储对文件、文件夹的操作事件
     */
    void pushEvent();

    /**
     * @brief popEvent 恢复对文件、文件夹的操作事件
     */
    void popEvent();

    /**
     * @brief getProcessOwner 获取当前程序启动的的系统用户名
     * @return  当前程序启动的的系统用户名
     */
    QString getProcessOwner();
private:
    QStack<DFMEvent> operatorStack;
    QMutex m_mtx;

    RevocationMgrInterface *m_dbusInterface = nullptr;

    RevocationEventType m_eventType = DFM_NO_EVENT;
};

DFM_END_NAMESPACE

#endif // OPERATORREVOCATION_H
