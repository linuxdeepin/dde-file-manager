// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGSYNCDEFS_H
#define CONFIGSYNCDEFS_H

#include <dfm-base/dfm_base_global.h>

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
