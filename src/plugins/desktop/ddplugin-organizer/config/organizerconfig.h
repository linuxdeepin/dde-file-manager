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

#endif // ORGANIZERCONFIG_H
