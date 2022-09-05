// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

    struct RevocationEvent{
        RevocationEventType eventType;
        QString user;
    };


    explicit RevocationManager(QObject *parent = nullptr);
    ~RevocationManager();

    static QString ObjectPath;

public slots:
    /**
     * @brief pushEvent 入栈事件
     * @param event 0:filemgr 1:desktop
     * @param1 user 事件执行用户
     */
    void pushEvent(int event, const QString& user);

    /**
     * @brief popEvent 弹出最新事件
     * @returns 事件 0:filemgr 1:desktop
     */
    int popEvent();

signals:
    /**
    * @brief fmgrRevocationAction 发送给文管的撤销动作(处理文管)
    * @param user 撤销动作的执行者
    */
    void fmgrRevocationAction(const QString& user);

    /**
    * @brief deskRevocationAction 发送给桌面的撤销动作(处理桌面)
    * @param user 撤销动作的执行者
    */
    void deskRevocationAction(const QString& user);

private:
    RevocationMgrAdaptor* m_adaptor = nullptr;
    QStack<RevocationEvent> m_eventStack;
};

#endif // REVOCATIONMANAGER_H
