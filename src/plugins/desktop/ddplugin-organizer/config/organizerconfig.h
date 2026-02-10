// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
    void setLastStyleConfigId(const QString &id);
    QString lastStyleConfigId() const;
    bool hasConfigId(const QString &configId) const;


    QList<CollectionBaseDataPtr> collectionBase(bool custom) const;
    CollectionBaseDataPtr collectionBase(bool custom, const QString &key) const;
    void updateCollectionBase(bool custom, const CollectionBaseDataPtr &base);
    void writeCollectionBase(bool custom, const QList<CollectionBaseDataPtr> &base);

    CollectionStyle collectionStyle(const QString &styleId, const QString &key) const;
    void updateCollectionStyle(const QString &styleId, const CollectionStyle &style);
    void writeCollectionStyle(const QString &styleId, const QList<CollectionStyle> &styles);

public:
signals:

public slots:
protected:
    QString path() const;

private:
    OrganizerConfigPrivate *d;
};

}

#endif   // ORGANIZERCONFIG_H
