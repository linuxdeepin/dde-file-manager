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

FileClassifier::FileClassifier(QObject *parent) : QObject(parent)
{

}

void FileClassifier::reset(const QList<QUrl> &urls)
{
    regionDatas.clear();
    QList<QUrl> temp;
    for (const QString &key : classes())
        regionDatas.insert(key, temp);

    for (const QUrl &url : urls) {
        auto type = classify(url);
        if (type.isEmpty()) {
            qWarning() << "can not find file:" << url;
            continue;
        }

        auto it = regionDatas.find(type);
        if (it != regionDatas.end())
            it->append(url);
        else
            Q_ASSERT_X(it == regionDatas.end(), "TypeClassifier", QString("unrecognized type %0").arg(type).toStdString().c_str());
    }
}

QString FileClassifier::repalce(const QUrl &oldUrl, const QUrl &newUrl)
{
    QString oldType = region(oldUrl);
    QString newType = classify(newUrl);
    QString newKey = region(newUrl);
#if 1
    //! the newUrl must be not existed,
    //! and the oldUrl must be existed,
    //! see FileProxyModelPrivate::sourceDataRenamed
    Q_ASSERT_X(newKey.isEmpty(), "FileClassifier", QString("newUrl is existed: %0").arg(newUrl.toString()).toStdString().c_str());
    Q_ASSERT_X(!oldType.isEmpty(), "FileClassifier", QString("oldUrl is not existed: %0").arg(oldUrl.toString()).toStdString().c_str());

    if (Q_UNLIKELY(newType.isEmpty())) {
        qWarning() << "can not find file:" << newUrl;
        regionDatas[oldType].removeOne(oldUrl);
        return newType;
    }

    if (oldType == newType) {
        int idx = regionDatas[newType].indexOf(oldUrl);
        regionDatas[newType].replace(idx, newUrl);
    } else {
        regionDatas[oldType].removeOne(oldUrl);
        regionDatas[newType].append(newUrl);
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

    QString cur = region(url);

    // do not exist
    if (cur.isEmpty()) {
        auto it = regionDatas.find(ret);
        if (it != regionDatas.end())
            it->append(url);
        else
            Q_ASSERT_X(it == regionDatas.end(), "TypeClassifier", QString("unrecognized type %0").arg(ret).toStdString().c_str());
    } else { // existed
        if (cur != ret) {
            regionDatas[cur].removeOne(url);
            regionDatas[ret].append(url);
        }
    }

    return ret;
}

QString FileClassifier::take(const QUrl &url)
{
    QString ret;
    for (auto itor = regionDatas.begin(); itor != regionDatas.end(); ++itor) {
        if (itor.value().contains(url)) {
            itor->removeOne(url);
            ret = itor.key();
            break;
        }
    }

    return ret;
}

bool FileClassifier::change(const QUrl &url)
{
    QString cur = region(url);
    if (cur.isEmpty())
        return false;

    QString ret = classify(url);
    if (ret != cur) {
        regionDatas[cur].removeOne(url);
        regionDatas[ret].append(url);
        return true;
    }
    return false;
}

QString FileClassifier::region(const QUrl &url) const
{
    QString ret;
    for (auto itor = regionDatas.begin(); itor != regionDatas.end(); ++itor) {
        if (itor.value().contains(url)) {
            ret = itor.key();
            break;
        }
    }

    return ret;
}

QList<QString> FileClassifier::regionKeys() const
{
    return regionDatas.keys();
}

QList<QUrl> FileClassifier::items(const QString &key) const
{
    return regionDatas.value(key);
}

