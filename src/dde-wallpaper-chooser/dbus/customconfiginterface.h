/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#ifndef CUSTOMCONFIGINTERFACE_H
#define CUSTOMCONFIGINTERFACE_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtDBus>

/*
 * Proxy class for interface com.deepin.CustomConfig
 */

class CustomConfigInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "com.deepin.CustomConfig"; }
public:
    CustomConfigInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);
    ~CustomConfigInterface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> StartCustomConfig(const QString &name)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(name);
        return asyncCallWithArgumentList(QStringLiteral("StartCustomConfig"), argumentList);
    }
};

namespace com {
  namespace deepin {
    typedef ::CustomConfigInterface CustomConfig;
  }
}

#endif // CUSTOMCONFIGINTERFACE_H
