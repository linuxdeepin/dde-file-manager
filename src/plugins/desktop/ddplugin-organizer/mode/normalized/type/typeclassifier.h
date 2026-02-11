// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TYPECLASSIFIER_H
#define TYPECLASSIFIER_H

#include "mode/normalized/fileclassifier.h"

#include <QSet>

namespace ddplugin_organizer {

class TypeClassifierPrivate;
class TypeClassifier : public FileClassifier
{
    Q_OBJECT
    friend class TypeClassifierPrivate;

public:
    explicit TypeClassifier(QObject *parent = nullptr);
    ~TypeClassifier();
    Classifier mode() const override;
    ModelDataHandler *dataHandler() const override;
    QStringList classes() const override;
    QString classify(const QUrl &) const override;
    QString className(const QString &key) const override;
    bool updateClassifier() override;

public:
    QString replace(const QUrl &oldUrl, const QUrl &newUrl) override;
    QString append(const QUrl &) override;
    QString prepend(const QUrl &) override;
    QString remove(const QUrl &) override;
    QString change(const QUrl &) override;

    // ModelDataHandler interface
public:
    virtual bool acceptRename(const QUrl &oldUrl, const QUrl &newUrl) override;

private:
    TypeClassifierPrivate *d;
    ModelDataHandler *handler = nullptr;
};
}

#endif   // TYPECLASSIFIER_H
