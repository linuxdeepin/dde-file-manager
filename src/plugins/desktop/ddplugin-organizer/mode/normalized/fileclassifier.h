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
#ifndef FILECLASSIFIER_H
#define FILECLASSIFIER_H

#include "ddplugin_organizer_global.h"
#include "organizer_defines.h"
#include "mode/collectiondataprovider.h"

#include <QObject>
#include <QHash>
#include <QUrl>

namespace ddplugin_organizer {

class ModelDataHandler;
class ClassifierCreator
{
public:
    static class FileClassifier *createClassifier(Classifier mode);
};

class FileClassifier : public CollectionDataProvider
{
    Q_OBJECT
public:
    explicit FileClassifier(QObject *parent = nullptr);
    virtual Classifier mode() const = 0;
    virtual ModelDataHandler *dataHandler() const = 0;
    virtual QStringList classes() const = 0;
    virtual QString classify(const QUrl &) const = 0;
    virtual QString className(const QString &) const = 0;
    virtual void reset(const QList<QUrl> &);
public:
    CollectionBaseDataPtr baseData(const QString &key) const;
    QList<CollectionBaseDataPtr> baseData() const;
public:
    QString replace(const QUrl &oldUrl, const QUrl &newUrl) override;
    QString append(const QUrl &) override;
    QString prepend(const QUrl &) override;
    void insert(const QUrl &, const QString &, const int) override;
    QString remove(const QUrl &) override;
    QString change(const QUrl &) override;
signals:
};

}

#endif // FILECLASSIFIER_H
