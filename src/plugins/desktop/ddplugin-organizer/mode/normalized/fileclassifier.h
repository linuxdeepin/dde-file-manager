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

#include <QObject>
#include <QHash>
#include <QUrl>

DDP_ORGANIZER_BEGIN_NAMESPACE

class ModelDataHandler;
class ClassifierCreator
{
public:
    enum Classifier {
        kType = 0,
        kTime,
        kLabel,
        kName,
        kSize
    };
    static class FileClassifier *createClassifier(Classifier mode);
};

class FileClassifier : public QObject
{
    Q_OBJECT
public:
    explicit FileClassifier(QObject *parent = nullptr);
    virtual int mode() const = 0;
    virtual QString name(const QString &id) const = 0;
    virtual ModelDataHandler *dataHandler() const = 0;
    virtual QStringList classes() const = 0;
    virtual QString classify(const QUrl &) const = 0;
public:
    virtual void reset(const QList<QUrl> &);
    virtual QString repalce(const QUrl &oldUrl, const QUrl &newUrl);
    virtual QString append(const QUrl &);
    virtual QString take(const QUrl &);
    virtual bool change(const QUrl &);
    virtual QString region(const QUrl &) const;
    virtual QList<QString> regionKeys() const;
    virtual QList<QUrl> items(const QString &key) const;
signals:

protected:
    QHash<QString, QList<QUrl>> regionDatas; // id -- files
};

DDP_ORGANIZER_END_NAMESPACE

#endif // FILECLASSIFIER_H
