// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTICALSHAREPROXY_H
#define OPTICALSHAREPROXY_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QScopedPointer>
#include <QVariantMap>

class QDBusInterface;

namespace dfmbase {

class OpticalShareProxy : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OpticalShareProxy)

public:
    static OpticalShareProxy &instance();

    QVariantMap burnState(const QString &dev);
    QVariantMap burnStates();
    bool setBurnState(const QString &dev, const QVariantMap &state);
    bool clearBurnState(const QString &dev);

    QVariantMap burnAttribute(const QString &tag);
    bool setBurnAttribute(const QString &tag, const QVariantMap &attribute);
    bool clearBurnAttribute(const QString &tag);

Q_SIGNALS:
    void burnStateChanged(const QString &dev, const QVariantMap &state);
    void burnAttributeChanged(const QString &tag, const QVariantMap &attribute);

private Q_SLOTS:
    void onBurnStateChanged(const QString &dev, const QVariantMap &state);
    void onBurnAttributeChanged(const QString &tag, const QVariantMap &attribute);

private:
    explicit OpticalShareProxy(QObject *parent = nullptr);

    QDBusInterface *dbusInterface();

private:
    QScopedPointer<QDBusInterface> interface;
};

}   // namespace dfmbase

#endif   // OPTICALSHAREPROXY_H
