// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "customdatahandler.h"

#include <QDebug>

using namespace ddplugin_organizer;

CustomDataHandler::CustomDataHandler(QObject *parent)
    : CollectionDataProvider(parent), ModelDataHandler()
{
}

CustomDataHandler::~CustomDataHandler()
{
}

void CustomDataHandler::check(const QSet<QUrl> &vaild)
{
    for (auto iter = collections.begin(); iter != collections.end(); ++iter) {
        for (auto it = iter.value()->items.begin(); it != iter.value()->items.end();) {
            if (vaild.contains(*it)) {
                ++it;
            } else {
                it = iter.value()->items.erase(it);
            }
        }
    }
}

QList<CollectionBaseDataPtr> CustomDataHandler::baseDatas() const
{
    return collections.values();
}

bool CustomDataHandler::addBaseData(const CollectionBaseDataPtr &base)
{
    if (!base) {
        fmWarning() << "Cannot add null collection base data";
        return false;
    }

    if (collections.contains(base->key)) {
        fmWarning() << "Collection with key already exists:" << base->key;
        return false;
    }

    collections.insert(base->key, base);
    return true;
}

void CustomDataHandler::removeBaseData(const QString &key)
{
    collections.remove(key);
}

bool CustomDataHandler::reset(const QList<CollectionBaseDataPtr> &datas)
{
    for (const CollectionBaseDataPtr &ptr : datas)
        collections.insert(ptr->key, ptr);

    return true;
}

QString CustomDataHandler::remove(const QUrl &url)
{
    for (auto iter = collections.begin(); iter != collections.end(); ++iter) {
        if (iter.value()->items.removeOne(url)) {
            emit itemsChanged(iter.key());
            return iter.key();
        }
    }

    return "";
}

QString CustomDataHandler::change(const QUrl &)
{
    return "";
}

QString CustomDataHandler::replace(const QUrl &oldUrl, const QUrl &newUrl)
{
    CollectionBaseDataPtr old;
    int oldIdx = -1;
    int newIdx = -1;
    for (auto iter = collections.begin(); iter != collections.end(); ++iter) {
        if (oldIdx < 0) {
            oldIdx = iter.value()->items.indexOf(oldUrl);
            if (oldIdx >= 0) {
                // find old
                old = iter.value();
            }
        }

        if (newIdx < 0)
            newIdx = iter.value()->items.indexOf(newUrl);

        if (oldIdx >= 0 && newIdx >= 0)
            break;
    }

    if (oldIdx < 0) {
        fmWarning() << "Replace failed - old URL not found:" << oldUrl;
        return "";
    }

    if (newIdx >= 0) {
        fmWarning() << "Replace failed - new URL already exists:" << newUrl;
        return "";
    }

    Q_ASSERT(old);
    old->items.replace(oldIdx, newUrl);
    emit itemsChanged(old->key);

    return old->key;
}

QString CustomDataHandler::append(const QUrl &)
{
    return "";
}

QString CustomDataHandler::prepend(const QUrl &)
{
    return "";
}

void CustomDataHandler::insert(const QUrl &url, const QString &key, const int index)
{
    auto it = collections.find(key);
    if (Q_UNLIKELY(it == collections.end())) {
        fmInfo() << "Creating new collection" << key << "for URL:" << url;
        CollectionBaseDataPtr base(new CollectionBaseData);
        base->key = key;
        base->items << url;
    } else {
        if (it.value()->items.size() < index || index < 0)
            it.value()->items.append(url);
        else
            it.value()->items.insert(index, url);
    }

    emit itemsChanged(key);
}

bool CustomDataHandler::acceptInsert(const QUrl &url)
{
    // todo(wcl) 新建流程

    for (auto iter = collections.begin(); iter != collections.end(); ++iter) {
        if (iter.value()->items.contains(url)) {
            return true;
        }
    }

    return false;
}

QList<QUrl> CustomDataHandler::acceptReset(const QList<QUrl> &urls)
{
    QList<QUrl> ret;
    for (const QUrl &url : urls) {
        for (auto iter = collections.begin(); iter != collections.end(); ++iter) {
            if (iter.value()->items.contains(url)) {
                ret << url;
            }
        }
    }

    return ret;
}

bool CustomDataHandler::acceptRename(const QUrl &oldUrl, const QUrl &newUrl)
{
    for (auto iter = collections.begin(); iter != collections.end(); ++iter) {
        if (iter.value()->items.contains(oldUrl)
            || iter.value()->items.contains(newUrl))
            return true;
    }

    return false;
}
