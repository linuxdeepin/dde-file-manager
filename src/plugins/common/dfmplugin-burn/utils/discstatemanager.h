// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISCSTATEMANAGER_H
#define DISCSTATEMANAGER_H

#include "dfmplugin_burn_global.h"

#include <QObject>

namespace dfmplugin_burn {

class DiscStateManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DiscStateManager)

public:
    static DiscStateManager *instance();

    void initilaize();

private slots:
    void ghostMountForBlankDisc();
    void onDevicePropertyChanged(const QString &id, const QString &propertyName, const QVariant &var);

private:
    explicit DiscStateManager(QObject *parent = nullptr);
};

}

#endif   // DISCSTATEMANAGER_H
