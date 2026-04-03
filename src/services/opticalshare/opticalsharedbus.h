// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTICALSHAREDBUS_H
#define OPTICALSHAREDBUS_H

#include <QObject>
#include <QDBusContext>
#include <QVariantMap>

class OpticalShareAdaptor;
class OpticalShareDBus : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.OpticalShare")

public:
    explicit OpticalShareDBus(const char *name, QObject *parent = nullptr);
    ~OpticalShareDBus();

public Q_SLOTS:
    Q_SCRIPTABLE QVariantMap GetBurnState(const QString &dev) const;
    Q_SCRIPTABLE QVariantMap GetBurnStates() const;
    Q_SCRIPTABLE bool SetBurnState(const QString &dev, const QVariantMap &state);
    Q_SCRIPTABLE bool ClearBurnState(const QString &dev);

    Q_SCRIPTABLE QVariantMap GetBurnAttribute(const QString &tag) const;
    Q_SCRIPTABLE bool SetBurnAttribute(const QString &tag, const QVariantMap &attribute);
    Q_SCRIPTABLE bool ClearBurnAttribute(const QString &tag);

Q_SIGNALS:
    void BurnStateChanged(const QString &dev, const QVariantMap &state);
    void BurnAttributeChanged(const QString &tag, const QVariantMap &attribute);

private:
    QVariantMap normalizeBurnState(const QVariantMap &state) const;
    QVariantMap normalizeBurnAttribute(const QVariantMap &attribute) const;
    qulonglong callerUid() const;

private:
    OpticalShareAdaptor *adapter { nullptr };
    QVariantMap burnStates;
    QVariantMap burnAttributes;
};

#endif   // OPTICALSHAREDBUS_H
