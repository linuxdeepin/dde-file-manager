// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHARECONTROLDBUS_H
#define SHARECONTROLDBUS_H

#include <QObject>
#include <QDBusContext>

class ShareControlAdapter;
class ShareControlDBus : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.filemanager.daemon.UserShareManager")

public:
    explicit ShareControlDBus(QObject *parent = nullptr);
    ~ShareControlDBus();

public slots:
    bool CloseSmbShareByShareName(const QString &name, bool show);
    bool SetUserSharePassword(const QString &name, const QString &passwd);
    bool EnableSmbServices();
    bool IsUserSharePasswordSet(const QString &username);

protected:
    bool checkAuthentication();

private:
    ShareControlAdapter *adapter = nullptr;
};

#endif   // SHARECONTROLDBUS_H
