/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "fileclassifier.h"
#include "type/typeclassifier.h"
#include "models/modeldatahandler.h"

#include <QDebug>

DDP_ORGANIZER_USE_NAMESPACE

FileClassifier *ClassifierCreator::createClassifier(Classifier mode)
{
    FileClassifier *ret = nullptr;
    switch (mode) {
    case Classifier::kType:
        ret = new TypeClassifier();
        break;
    default:
        break;
    }
    return ret;
}

FileClassifier::FileClassifier(QObject *parent) : CollectionDataProvider(parent)
{

}

void FileClassifier::reset(const QList<QUrl> &urls)
{
    collections.clear();
    for (const QString &id : classes()) {
        CollectionBaseDataPtr dp(new CollectionBaseData);
        dp->name = className(id);
        dp->key = id;

        collections.insert(id, dp);
    }

    for (const QUrl &url : urls) {
        auto type = classify(url);
        if (type.isEmpty()) {
            qWarning() << "can not find file:" << url;
            continue;
        }

        auto it = collections.find(type);
        if (it != collections.end())
            it.value()->items.append(url);
        else
            Q_ASSERT_X(it == collections.end(), "TypeClassifier", QString("unrecognized type %0").arg(type).toStdString().c_str());
    }
}

QString FileClassifier::replace(const QUrl &oldUrl, const QUrl &newUrl)
{
    QString oldType = key(oldUrl);
    QString newType = classify(newUrl);
    QString newKey = key(newUrl);
#if 1
    //! the newUrl must be not existed,
    //! and the oldUrl must be existed,
    //! see FileProxyModelPrivate::sourceDataRenamed
    Q_ASSERT_X(newKey.isEmpty(), "FileClassifier", QString("newUrl is existed: %0").arg(newUrl.toString()).toStdString().c_str());
    Q_ASSERT_X(!oldType.isEmpty(), "FileClassifier", QString("oldUrl is not existed: %0").arg(oldUrl.toString()).toStdString().c_str());

    if (Q_UNLIKELY(newType.isEmpty())) {
        qWarning() << "can not find file:" << newUrl;
        collections[oldType]->items.removeOne(oldUrl);
        return newType;
    }

    if (oldType == newType) {
        int idx = collections[newType]->items.indexOf(oldUrl);
        collections[newType]->items.replace(idx, newUrl);
    } else {
        collections[oldType]->items.removeOne(oldUrl);
        collections[newType]->items.append(newUrl);
    }
#else
    // old does not exist.
    if (oldType.isEmpty()) {
        if (Q_UNLIKELY(newType.isEmpty())) {
            qWarning() << "can not find file:" << newUrl;
            return newType;
        }

        // newer does not exist
        if (newKey.isEmpty()) {
            regionDatas[newType].append(newUrl);
        } else { // newer is existed
            if (newType != newKey) {
                regionDatas[newKey].removeOne(newUrl);
                regionDatas[newType].append(newUrl);
            }
        }
    } else {
        if (Q_UNLIKELY(newType.isEmpty())) {
            qWarning() << "can not find file:" << newUrl;
            regionDatas[oldType].removeOne(oldUrl);
            return newType;
        }

        // newer does not exist
        if (newKey.isEmpty()) {
            if (oldType == newType) {
                int idx = regionDatas[newType].indexOf(oldUrl);
                regionDatas[newType].replace(idx, newUrl);
            } else {
                regionDatas[oldType].removeOne(oldUrl);
                regionDatas[newType].append(newUrl);
            }
        } else { // newer is existed
            regionDatas[oldType].removeOne(oldUrl);
            if (newType != newKey) {
                regionDatas[newKey].removeOne(newUrl);
                regionDatas[newType].append(newUrl);
            }
        }
    }
#endif
    return newType;
}

QString FileClassifier::append(const QUrl &url)
{
    QString ret = classify(url);
    if (ret.isEmpty()) {
        qWarning() << "can not find file:" << url;
        return ret;
    }

    QString cur = key(url);

    // do not exist
    if (cur.isEmpty()) {
        auto it = collections.find(ret);
        if (it != collections.end())
            it.value()->items.append(url);
        else
            Q_ASSERT_X(it == collections.end(), "TypeClassifier", QString("unrecognized type %0").arg(ret).toStdString().c_str());
    } else { // existed
        if (cur != ret) {
            collections[cur]->items.removeOne(url);
            collections[ret]->items.append(url);
        }
    }

    return ret;
}

QString FileClassifier::remove(const QUrl &url)
{
    QString ret;
    for (auto itor = collections.begin(); itor != collections.end(); ++itor) {
        if (itor.value()->items.contains(url)) {
            itor.value()->items.removeOne(url);
            ret = itor.key();
            break;
        }
    }

    return ret;
}

QString FileClassifier::change(const QUrl &url)
{
    QString cur = key(url);
    if (cur.isEmpty())
        return "";

    QString ret = classify(url);
    if (ret != cur) {
        collections[cur]->items.removeOne(url);
        collections[ret]->items.append(url);
        return ret;
    }
    return "";
}



