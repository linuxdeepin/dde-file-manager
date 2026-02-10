// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORGANIZERCONFIG_P_H
#define ORGANIZERCONFIG_P_H

#include "organizerconfig.h"

#include <QSettings>
#include <QTimer>

namespace ddplugin_organizer {

class OrganizerConfigPrivate
{
public:
    explicit OrganizerConfigPrivate(OrganizerConfig *);
    ~OrganizerConfigPrivate();
    QVariant value(const QString &group, const QString &key, const QVariant &defaultVar);
    void setValue(const QString &group, const QString &key, const QVariant &var);
    QSettings *settings = nullptr;
    QTimer syncTimer;
private:
    OrganizerConfig *q;
};

}

#endif // ORGANIZERCONFIG_P_H
