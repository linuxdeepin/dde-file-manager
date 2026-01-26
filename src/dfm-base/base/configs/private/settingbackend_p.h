// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGBACKEND_P_H
#define SETTINGBACKEND_P_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/base/application/application.h>

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
