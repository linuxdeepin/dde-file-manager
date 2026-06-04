// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "opticalsharedbus.h"

#include "daemonplugin_opticalshare_global.h"
#include "opticalshareadaptor.h"

#include <dfm-base/dbusservice/global_server_defines.h>

#include <QDBusConnection>
#include <QDateTime>

DAEMONPOPTICALSHARE_USE_NAMESPACE

using namespace GlobalServerDefines;

OpticalShareDBus::OpticalShareDBus(QObject *parent)
    : QObject(parent), QDBusContext()
{
    fmInfo() << "OpticalShareDBus: instance created";
    adapter = new OpticalShareAdaptor(this);
}

OpticalShareDBus::~OpticalShareDBus()
{
    fmInfo() << "OpticalShareDBus: service destroyed";
}

QVariantMap OpticalShareDBus::GetBurnState(const QString &dev) const
{
    return burnStates.value(dev).toMap();
}

QVariantMap OpticalShareDBus::GetBurnStates() const
{
    return burnStates;
}

bool OpticalShareDBus::SetBurnState(const QString &dev, const QVariantMap &state)
{
    if (dev.isEmpty()) {
        fmWarning() << "OpticalShareDBus::SetBurnState: empty device";
        return false;
    }

    const QVariantMap normalized = normalizeBurnState(state);
    burnStates.insert(dev, normalized);
    emit BurnStateChanged(dev, normalized);
    return true;
}

bool OpticalShareDBus::ClearBurnState(const QString &dev)
{
    if (dev.isEmpty())
        return false;

    QVariantMap previousState = burnStates.value(dev).toMap();
    burnStates.remove(dev);

    if (!previousState.isEmpty()) {
        previousState[OpticalShareField::kWorking] = false;
        previousState[OpticalShareField::kTimestamp] = QDateTime::currentSecsSinceEpoch();
    }

    emit BurnStateChanged(dev, previousState);
    return true;
}

QVariantMap OpticalShareDBus::GetBurnAttribute(const QString &tag) const
{
    return burnAttributes.value(tag).toMap();
}

bool OpticalShareDBus::SetBurnAttribute(const QString &tag, const QVariantMap &attribute)
{
    if (tag.isEmpty()) {
        fmWarning() << "OpticalShareDBus::SetBurnAttribute: empty tag";
        return false;
    }

    const QVariantMap normalized = normalizeBurnAttribute(attribute);
    burnAttributes.insert(tag, normalized);
    emit BurnAttributeChanged(tag, normalized);
    return true;
}

bool OpticalShareDBus::ClearBurnAttribute(const QString &tag)
{
    if (tag.isEmpty())
        return false;

    burnAttributes.remove(tag);
    emit BurnAttributeChanged(tag, QVariantMap());
    return true;
}

QVariantMap OpticalShareDBus::normalizeBurnState(const QVariantMap &state) const
{
    QVariantMap normalized = state;
    normalized[OpticalShareField::kTimestamp] = QDateTime::currentSecsSinceEpoch();
    return normalized;
}

QVariantMap OpticalShareDBus::normalizeBurnAttribute(const QVariantMap &attribute) const
{
    QVariantMap normalized = attribute;
    normalized[OpticalShareField::kTimestamp] = QDateTime::currentSecsSinceEpoch();
    return normalized;
}
