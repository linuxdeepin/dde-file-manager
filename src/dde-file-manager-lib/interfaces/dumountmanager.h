// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

