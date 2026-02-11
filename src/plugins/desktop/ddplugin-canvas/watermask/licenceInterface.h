// SPDX-FileCopyrightText: 2020 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LICENCEINTERFACE_H
#define LICENCEINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface com.deepin.license
 */
class ComDeepinLicenseInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "com.deepin.license.Info"; }

public:
    ComDeepinLicenseInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    ~ComDeepinLicenseInterface();

    Q_PROPERTY(int AuthorizationState READ authorizationState)
    inline int authorizationState() const
    { return qvariant_cast< int >(property("AuthorizationState")); }

    Q_PROPERTY(uint AuthorizationProperty READ authorizationProperty)
    inline uint authorizationProperty() const
    { return qvariant_cast< uint >(property("AuthorizationProperty")); }

    Q_PROPERTY(uint ServiceProperty READ serviceProperty)
    inline uint serviceProperty() const
    { return qvariant_cast< uint >(property("ServiceProperty")); }

Q_SIGNALS: // SIGNALS
    void LicenseStateChange();

};

namespace com {
  namespace deepin {
    namespace license {
      typedef ::ComDeepinLicenseInterface Info;
    }
  }
}

#endif // LICENCEINTERFACE_H
