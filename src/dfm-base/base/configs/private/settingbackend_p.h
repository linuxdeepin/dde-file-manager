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
#ifndef SETTINGBACKEND_P_H
#define SETTINGBACKEND_P_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/base/application/application.h"

#include <QMap>
#include <QVariant>
#include <QSet>

DFMBASE_BEGIN_NAMESPACE

using GetOptFunc = std::function<QVariant()>;
using SaveOptFunc = std::function<void(const QVariant &)>;

template<typename Key, typename T>
class BidirectionHash
{
public:
#ifdef Q_COMPILER_INITIALIZER_LISTS
    inline BidirectionHash(std::initializer_list<std::pair<Key, T>> list)
    {
        k2v.reserve(int(list.size()));
        v2k.reserve(int(list.size()));

        for (auto it = list.begin(); it != list.end(); ++it) {
            k2v.insert(it->first, it->second);
            v2k.insert(it->second, it->first);
        }
    }
#endif

    inline bool containsKey(const Key &key) const
    {
        return k2v.contains(key);
    }
    inline bool containsValue(const T &value) const
    {
        return v2k.contains(value);
    }
    inline const Key key(const T &value) const
    {
        return v2k.value(value);
    }
    inline const Key key(const T &value, const Key &defaultKey) const
    {
        return v2k.value(value, defaultKey);
    }
    inline const T value(const Key &key) const
    {
        return k2v.value(key);
    }
    inline const T value(const Key &key, const T &defaultValue) const
    {
        return k2v.value(key, defaultValue);
    }
    inline QList<Key> keys() const
    {
        return k2v.keys();
    }

private:
    QHash<Key, T> k2v;
    QHash<T, Key> v2k;
};

class SettingBackend;
class SettingBackendPrivate
{
    friend SettingBackend;

private:
    void saveAsAppAttr(const QString &key, const QVariant &val);
    QVariant getAsAppAttr(const QString &key);
    void saveAsGenAttr(const QString &key, const QVariant &val);
    QVariant getAsGenAttr(const QString &key);
    void saveByFunc(const QString &key, const QVariant &val);
    QVariant getByFunc(const QString &key);

private:
    QMap<QString, GetOptFunc> getters;
    QMap<QString, SaveOptFunc> setters;
    QSet<QString> serialDataKey;

    static BidirectionHash<QString, Application::ApplicationAttribute> keyToAA;
    static BidirectionHash<QString, Application::GenericAttribute> keyToGA;
};

DFMBASE_END_NAMESPACE

#endif   // SETTINGBACKEND_P_H
