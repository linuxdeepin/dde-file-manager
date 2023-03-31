// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGSYNCHRONIZER_P_H
#define CONFIGSYNCHRONIZER_P_H

#include <dfm-base/dfm_base_global.h>

#include <dfm-base/base/configs/configsyncdefs.h>
#include <dfm-base/base/application/application.h>

#include <QRegularExpression>
#include <QSet>

DFMBASE_BEGIN_NAMESPACE

class ConfigSynchronizer;
class ConfigSynchronizerPrivate
{
    friend class ConfigSynchronizer;

public:
    explicit ConfigSynchronizerPrivate(ConfigSynchronizer *qq);

    void initConn();
    void onDConfChanged(const QString &cfgPath, const QString &cfgKey);
    void syncToAppSet(const QString &cfgPath, const QString &cfgKey, const QVariant &var);

private:
    ConfigSynchronizer *q { nullptr };
    QHash<QString, SyncPair> syncPairs;
};

DFMBASE_END_NAMESPACE

#endif   // CONFIGSYNCHRONIZER_P_H
