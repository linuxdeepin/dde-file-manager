/**
 ** This file is part of the filemanager project.
 ** Copyright 2020 luzhen <luzhen@uniontech.com>.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef VAULTCLOCK_H
#define VAULTCLOCK_H

#include <QObject>
#include <QTimer>

/**
 * @brief The VaultClock class
 */
class VaultClock : public QObject
{
    Q_OBJECT
public:
    explicit VaultClock(QObject *parent = nullptr);
    ~VaultClock();

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

    /**
     * @brief getSelfTime 获取自定义时间
     * @return
     */
    quint64 getSelfTime() const;

protected:
    /**
     * @brief tick 秒针
     */
    void tick();

private:
    quint64 m_lastestTime = 0; // latest time

    QTimer m_selfTimer;
    quint64 m_selfTime;
};

#endif // VAULTCLOCK_H
