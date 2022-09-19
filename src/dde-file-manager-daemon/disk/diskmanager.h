// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <QDBusContext>

class DiskAdaptor;

class DiskManager : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    explicit DiskManager(QObject *parent = nullptr);

    static QString ObjectPath;
    static QString PolicyKitActionId;

Q_SIGNALS:
    void finished(int code);
    void passwordChecked(bool result);

public Q_SLOTS:
    bool checkAuthentication();
    void changeDiskPassword(const QString &oldPwd, const QString &newPwd);

private:
    enum ResultType {
        Unknown = 0,
        Success,
        PasswordWrong,
        AccessDiskFailed,   // Unable to get the encrypted disk list
        PasswordInconsistent,   // Passwords of disks are different
        InitFailed   // Initialization failed
    };

    int changePassword(const char *oldPwd, const char *newPwd, const char *device);
    QStringList encryptedDisks();

private:
    DiskAdaptor *m_diskAdaptor = nullptr;
    bool m_isRestore = false;
};

#endif   // DISKMANAGER_H
