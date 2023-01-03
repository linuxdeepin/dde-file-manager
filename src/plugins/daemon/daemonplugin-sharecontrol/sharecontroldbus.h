/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
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
