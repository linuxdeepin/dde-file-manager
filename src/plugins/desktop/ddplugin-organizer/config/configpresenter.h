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

namespace ddplugin_organizer {
class OrganizerConfig;
class ConfigPresenter : public QObject
{
    Q_OBJECT
public:
    static ConfigPresenter* instance();
    bool initialize();
public:
    inline bool isEnable() const {return enable;}
    void setEnable(bool e);

    inline OrganizerMode mode() const {return curMode;}
    void setMode(OrganizerMode m);

    inline DisplaySize displaySize() const {return curDisplaySize;}
    inline void setDisplaySize(DisplaySize size) {curDisplaySize = size;}

    inline Classifier classification() const {return curClassifier;}
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
public:
    ItemCategories enabledTypeCategories() const;
    void setEnabledTypeCategories(ItemCategories flags);
signals:
    // use Qt::QueuedConnection
    void changeEnableState(bool e);
    void switchToNormalized(int);
    void switchToCustom();
    void changeDisplaySize(int);
    void newCollection(const QList<QUrl> &);
    void showOptionWindow();
public slots:
protected:
    explicit ConfigPresenter(QObject *parent = nullptr);
    ~ConfigPresenter() override;
private:
    OrganizerConfig *conf = nullptr;
    bool enable = false;
    OrganizerMode curMode = OrganizerMode::kNormalized;
    DisplaySize curDisplaySize = DisplaySize::kSmaller;
    Classifier curClassifier = Classifier::kType;
};

}

#define CfgPresenter ConfigPresenter::instance()
#endif // CONFIGPRESENTER_H
