// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORGANIZERCONFIG_H
#define ORGANIZERCONFIG_H

#include "ddplugin_organizer_global.h"
#include "organizer_defines.h"

#include <QObject>

namespace ddplugin_organizer {

class OrganizerConfigPrivate;
class OrganizerConfig : public QObject
{
    Q_OBJECT
    friend class OrganizerConfigPrivate;

public:
    explicit OrganizerConfig(QObject *parent = nullptr);
    ~OrganizerConfig() override;
    bool isEnable() const;
    void setEnable(bool e);
    int mode() const;
    void setMode(int m);
    void setVersion(const QString &v);
    QList<QSize> surfaceSizes();
    void setScreenInfo(const QMap<QString, QString> info);
    void sync(int ms = 1000);
    int classification() const;
    void setClassification(int cf);

    QList<CollectionBaseDataPtr> collectionBase(bool custom) const;
    CollectionBaseDataPtr collectionBase(bool custom, const QString &key) const;
    void updateCollectionBase(bool custom, const CollectionBaseDataPtr &base);
    void writeCollectionBase(bool custom, const QList<CollectionBaseDataPtr> &base);

    CollectionStyle collectionStyle(bool custom, const QString &key) const;
    void updateCollectionStyle(bool custom, const CollectionStyle &style);
    void writeCollectionStyle(bool custom, const QList<CollectionStyle> &styles);

public:
    int enabledTypeCategories() const;
    void setEnabledTypeCategories(int flags);
signals:

public slots:
protected:
    QString path() const;

private:
    OrganizerConfigPrivate *d;
};

}

#endif   // ORGANIZERCONFIG_H
