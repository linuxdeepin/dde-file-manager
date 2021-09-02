/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#ifndef DUMOUNT_MANAGER_H
#define DUMOUNT_MANAGER_H

#include <QObject>
#include <QDBusError>

class DefenderInterface;

class DUMountManager : public QObject
{
    Q_OBJECT

public:
    DUMountManager(QObject *parent = nullptr);
    ~DUMountManager();

    QString getDriveName(const QString &blkName);

    bool isScanningBlock(const QString &blkName);
    bool isScanningDrive(const QString &driveName = QString(""));
    bool stopScanBlock(const QString &blkName);
    bool stopScanDrive(const QString &driveName);
    bool stopScanAllDrive();

    bool umountBlock(const QString &blkName);
    bool umountBlocksOnDrive(const QString &driveName);
    bool removeDrive(const QString &driveName);
    bool ejectDrive(const QString &driveName);
    bool ejectAllDrive();

    QString getErrorMsg();

private:
    QString checkMountErrorMsg(const QDBusError &dbsError);
    QString checkEjectErrorMsg(const QDBusError &dbsError);
    QUrl getMountPathForBlock(const QString &blkName);
    QList<QUrl> getMountPathForDrive(const QString &driveName);
    QList<QUrl> getMountPathForAllDrive();
    void clearError();

private:
    QScopedPointer<DefenderInterface> m_defenderInterface;
    QString errorMsg;
};

#endif //DUMOUNT_MANAGER_H

