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
#ifndef CONFIGSYNCDEFS_H
#define CONFIGSYNCDEFS_H

#include "dfm_base_global.h"

#include <QObject>

#include <functional>

DFMBASE_BEGIN_NAMESPACE

using ConfigSaver = std::function<void(const QVariant &)>;
using SyncToAppSet = std::function<void(const QString &, const QString &, const QVariant &)>;
using IsConfEqual = std::function<bool(const QVariant &dconfVal, const QVariant &dsetVal)>;

enum SettingType {
    kNone = -1,
    kAppAttr,
    kGenAttr,
};

struct AppSetItem
{
    SettingType type { kNone };
    int val { -1 };
};
struct ConfigItem
{
    QString path;
    QString key;
};

struct SyncPair
{
    AppSetItem set;
    ConfigItem cfg;

    ConfigSaver saver { nullptr };
    SyncToAppSet toAppSet { nullptr };
    IsConfEqual isEqual { nullptr };

    inline bool isValid() const
    {
        return set.type != kNone && !cfg.path.isEmpty() && !cfg.key.isEmpty();
    }
    inline QString serialize() const
    {
        return serialize(set, cfg);
    }
    static inline QString serialize(const AppSetItem &set, const ConfigItem &cfg)
    {
        if (set.type == kNone)
            return QString(":%1/%2").arg(cfg.path).arg(cfg.key);
        if (cfg.key.isEmpty())
            return QString("%1/%2:").arg(set.type).arg(set.val);
        return QString("%1/%2:%3/%4").arg(set.type).arg(set.val).arg(cfg.path).arg(cfg.key);
    }
};

DFMBASE_END_NAMESPACE

#endif   // CONFIGSYNCDEFS_H
