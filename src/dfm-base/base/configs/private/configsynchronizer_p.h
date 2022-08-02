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
#ifndef CONFIGSYNCHRONIZER_P_H
#define CONFIGSYNCHRONIZER_P_H

#include "dfm_base_global.h"

#include "dfm-base/base/configs/configsyncdefs.h"
#include "dfm-base/base/application/application.h"

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
