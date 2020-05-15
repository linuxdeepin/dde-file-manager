/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef VAULTMANAGER_H
#define VAULTMANAGER_H

#include <QDBusContext>
#include <QObject>

class VaultAdaptor;

/**
 * @brief The VaultManager class 保险箱管理类
 */
class VaultManager : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    explicit VaultManager(QObject *parent = 0);
    ~VaultManager();

    static QString ObjectPath;
    static QString PolicyKitActionId;

public slots:
    /**
     * @brief setRefreshTime 设置保险箱刷新时间
     * @param time
     */
    void setRefreshTime(quint64 time);

    /**
     * @brief getLastestTime 获取保险柜计时
     * @return
     */
    quint64 getLastestTime() const;

private:
    VaultAdaptor* m_vaultAdaptor = nullptr;
    quint64 m_lastestTime = 0; // 保险箱最新计时
};

#endif // VAULTMANAGER_H
