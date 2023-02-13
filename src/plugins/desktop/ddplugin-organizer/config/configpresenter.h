// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
