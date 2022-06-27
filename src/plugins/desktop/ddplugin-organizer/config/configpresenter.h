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
#ifndef CONFIGPRESENTER_H
#define CONFIGPRESENTER_H

#include "ddplugin_organizer_global.h"
#include "organizer_defines.h"

#include <QObject>

DDP_ORGANIZER_BEGIN_NAMESPACE
class OrganizerConfig;
class ConfigPresenter : public QObject
{
    Q_OBJECT
public:
    static ConfigPresenter* instance();
    bool initialize();
    bool isEnable() const;
    void setEnable(bool e);
    OrganizerMode mode() const;
    void setMode(OrganizerMode m);
    Classifier classification() const;
    void setClassification(Classifier cf);
    QList<CollectionBaseDataPtr> customProfile() const;
    void saveCustomProfile(const QList<CollectionBaseDataPtr> &baseDatas);

    QList<CollectionBaseDataPtr> normalProfile() const;
    void saveNormalProfile(const QList<CollectionBaseDataPtr> &baseDatas);

    CollectionStyle normalStyle(const QString &key) const;
    void updateNormalStyle(const CollectionStyle &style) const;
    void writeNormalStyle(const QList<CollectionStyle> &styles) const;

    CollectionStyle customStyle(const QString &key) const;
    void updateCustomStyle(const CollectionStyle &style) const;
    void writeCustomStyle(const QList<CollectionStyle> &styles) const;
signals:
    void changeEnableState(bool e);
    void switchToNormalized(int);
    void switchToCustom();
    void newCollection(const QList<QUrl> &);
public slots:
protected:
    explicit ConfigPresenter(QObject *parent = nullptr);
    ~ConfigPresenter() override;
private:
    OrganizerConfig *conf = nullptr;
    bool enable = false;
    OrganizerMode curMode = OrganizerMode::kNormalized;
    Classifier curClassifier = Classifier::kType;
};

DDP_ORGANIZER_END_NAMESPACE

#define CfgPresenter ConfigPresenter::instance()
#endif // CONFIGPRESENTER_H
