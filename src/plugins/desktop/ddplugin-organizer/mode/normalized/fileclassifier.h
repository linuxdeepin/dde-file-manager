// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILECLASSIFIER_H
#define FILECLASSIFIER_H

#include "ddplugin_organizer_global.h"
#include "organizer_defines.h"
#include "mode/collectiondataprovider.h"
#include "models/modeldatahandler.h"

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

class FileClassifier : public CollectionDataProvider, public ModelDataHandler
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
    virtual bool updateClassifier() = 0;   // return true if changed

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

public:
    bool acceptInsert(const QUrl &url) override;
    bool acceptRename(const QUrl &oldUrl, const QUrl &newUrl) override;
};

}

#endif   // FILECLASSIFIER_H
