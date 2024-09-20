// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHARECONTROLDBUS_H
#define SHARECONTROLDBUS_H

#include <QObject>
#include <QDBusContext>

class UserShareManagerAdaptor;
class ShareControlDBus : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.UserShareManager")

public:
    explicit ShareControlDBus(const char *name, QObject *parent = nullptr);
    ~ShareControlDBus();

public slots:
    Q_SCRIPTABLE bool CloseSmbShareByShareName(const QString &name, bool show);
    Q_SCRIPTABLE bool SetUserSharePassword(const QString &name, const QString &passwd);
    Q_SCRIPTABLE bool EnableSmbServices();
    Q_SCRIPTABLE bool IsUserSharePasswordSet(const QString &username);

protected:
    bool checkAuthentication();

private:
    UserShareManagerAdaptor *adapter = nullptr;
};

#endif   // SHARECONTROLDBUS_H
