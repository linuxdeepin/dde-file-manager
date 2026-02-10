// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileclassifier.h"
#include "type/typeclassifier.h"
#include "models/modeldatahandler.h"
#include "config/configpresenter.h"

#include <QDebug>

using namespace ddplugin_organizer;

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

FileClassifier::FileClassifier(QObject *parent)
    : CollectionDataProvider(parent)
{
    connect(this, &FileClassifier::itemsChanged, this, [=]() {
        CfgPresenter->saveNormalProfile(baseData());
    });
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
            fmWarning() << "can not find file:" << url;
            continue;
        }

        auto it = collections.find(type);
        if (it != collections.end())
            it.value()->items.append(url);
        else
            Q_ASSERT_X(it == collections.end(), "TypeClassifier", QString("unrecognized type %0").arg(type).toStdString().c_str());
    }
}

QList<CollectionBaseDataPtr> FileClassifier::baseData() const
{
    return collections.values();
}

CollectionBaseDataPtr FileClassifier::baseData(const QString &key) const
{
    return collections.value(key);
}

QString FileClassifier::replace(const QUrl &oldUrl, const QUrl &newUrl)
{
    QString oldType = key(oldUrl);
    QString newType = classify(newUrl);
    QString newKey = key(newUrl);
#if 1
    //! the newUrl must be not existed,
    //! and the oldUrl must be existed,
    //! see CollectionModelPrivate::sourceDataRenamed
    Q_ASSERT_X(newKey.isEmpty(), "FileClassifier", QString("newUrl is existed: %0").arg(newUrl.toString()).toStdString().c_str());
    Q_ASSERT_X(!oldType.isEmpty(), "FileClassifier", QString("oldUrl is not existed: %0").arg(oldUrl.toString()).toStdString().c_str());

    if (Q_UNLIKELY(newType.isEmpty())) {
        fmWarning() << "can not find file:" << newUrl;
        collections[oldType]->items.removeOne(oldUrl);
        return newType;
    }

    if (oldType == newType) {
        int idx = collections[newType]->items.indexOf(oldUrl);
        collections[newType]->items.replace(idx, newUrl);
        emit itemsChanged(newType);
    } else {
        collections[oldType]->items.removeOne(oldUrl);
        emit itemsChanged(oldType);

        collections[newType]->items.append(newUrl);
        emit itemsChanged(newType);
    }
#else
    // old does not exist.
    if (oldType.isEmpty()) {
        if (Q_UNLIKELY(newType.isEmpty())) {
            fmWarning() << "can not find file:" << newUrl;
            return newType;
        }

        // newer does not exist
        if (newKey.isEmpty()) {
            regionDatas[newType].append(newUrl);
        } else {   // newer is existed
            if (newType != newKey) {
                regionDatas[newKey].removeOne(newUrl);
                regionDatas[newType].append(newUrl);
            }
        }
    } else {
        if (Q_UNLIKELY(newType.isEmpty())) {
            fmWarning() << "can not find file:" << newUrl;
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
        } else {   // newer is existed
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
        fmWarning() << "can not find file:" << url;
        return ret;
    }

    QString cur = key(url);

    // do not exist
    if (cur.isEmpty()) {
        auto it = collections.find(ret);
        if (it != collections.end()) {
            it.value()->items.append(url);
            emit itemsChanged(ret);
        } else {
            Q_ASSERT_X(it == collections.end(), "TypeClassifier", QString("unrecognized type %0").arg(ret).toStdString().c_str());
        }
    } else {   // existed
        if (cur != ret) {
            collections[cur]->items.removeOne(url);
            emit itemsChanged(cur);

            collections[ret]->items.append(url);
            emit itemsChanged(ret);
        }
    }

    return ret;
}

QString FileClassifier::prepend(const QUrl &url)
{
    QString ret = classify(url);
    if (ret.isEmpty()) {
        fmWarning() << "can not find file:" << url;
        return ret;
    }

    QString cur = key(url);

    // do not exist
    if (cur.isEmpty()) {
        auto it = collections.find(ret);
        if (it != collections.end()) {
            it.value()->items.prepend(url);
            emit itemsChanged(ret);
        } else {
            Q_ASSERT_X(it == collections.end(), "TypeClassifier", QString("unrecognized type %0").arg(ret).toStdString().c_str());
        }
    } else {   // existed
        if (cur != ret) {
            collections[cur]->items.removeOne(url);
            emit itemsChanged(cur);

            collections[ret]->items.prepend(url);
            emit itemsChanged(ret);
        }
    }

    return ret;
}

void FileClassifier::insert(const QUrl &, const QString &, const int)
{
    // todo
}

QString FileClassifier::remove(const QUrl &url)
{
    QString ret;
    for (auto itor = collections.begin(); itor != collections.end(); ++itor) {
        if (itor.value()->items.contains(url)) {
            itor.value()->items.removeOne(url);
            ret = itor.key();
            emit itemsChanged(ret);
            break;
        }
    }

    return ret;
}

QString FileClassifier::change(const QUrl &url)
{
    QString cur = key(url);
    if (cur.isEmpty()) {
        fmDebug() << "URL not found in any collection:" << url;
        return "";
    }

    QString ret = classify(url);
    if (ret != cur) {
        collections[cur]->items.removeOne(url);
        emit itemsChanged(cur);

        collections[ret]->items.append(url);
        emit itemsChanged(ret);

        return ret;
    }
    return "";
}

bool FileClassifier::acceptInsert(const QUrl &url)
{
    const auto type { classify(url) };
    return classes().contains(type);
}

bool FileClassifier::acceptRename(const QUrl &oldUrl, const QUrl &newUrl)
{
    Q_UNUSED(oldUrl);
    const auto type { classify(newUrl) };
    return classes().contains(type);
}
