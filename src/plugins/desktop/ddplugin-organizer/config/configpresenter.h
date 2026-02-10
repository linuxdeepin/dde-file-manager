// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
    static ConfigPresenter *instance();
    bool initialize();

public:
    inline QString version() const { return confVersion; }
    void setVersion(const QString &v);

    QList<QSize> surfaceSizes();
    void setSurfaceInfo(const QList<QWidget *> surfaces);

    void setLastStyleConfigId(const QString &id);
    QString lastStyleConfigId() const;

    bool hasConfigId(const QString &configId) const;

    inline bool isEnable() const { return enable; }
    void setEnable(bool e);

    inline bool isEnableVisibility() const { return enableVisibility; }
    void setEnableVisibility(bool v);

    inline OrganizerMode mode() const { return curMode; }
    void setMode(OrganizerMode m);

    inline Classifier classification() const { return curClassifier; }
    void setClassification(Classifier cf);

    QKeySequence hideAllKeySequence() const;
    void setHideAllKeySequence(const QKeySequence &seq);

    bool isRepeatNoMore() const;
    void setRepeatNoMore(bool e);

    QList<CollectionBaseDataPtr> customProfile() const;
    void saveCustomProfile(const QList<CollectionBaseDataPtr> &baseDatas);

    QList<CollectionBaseDataPtr> normalProfile() const;
    void saveNormalProfile(const QList<CollectionBaseDataPtr> &baseDatas);

    CollectionStyle normalStyle(const QString &configId, const QString &key) const;
    void updateNormalStyle(const QString &configId, const CollectionStyle &style) const;
    void writeNormalStyle(const QString &configId, const QList<CollectionStyle> &styles) const;

    CollectionStyle customStyle(const QString &key) const;
    void updateCustomStyle(const CollectionStyle &style) const;
    void writeCustomStyle(const QList<CollectionStyle> &styles) const;

public:
    ItemCategories enabledTypeCategories() const;
    void setEnabledTypeCategories(ItemCategories flags);

    OrganizeAction organizeAction() const;
    bool organizeOnTriggered() const;

    bool optimizeMovingPerformance() const;

signals:
    // use Qt::QueuedConnection
    void changeEnableState(bool e);
    void changeEnableVisibilityState(bool v);
    void changeHideAllKeySequence(const QKeySequence &);
    void switchToNormalized(int);
    void releaseCollection(int category);
    void switchToCustom();
    void changeDisplaySize(int);
    void newCollection(const QList<QUrl> &);
    void showOptionWindow();
    void reorganizeDesktop();
    void optimizeStateChanged(bool);

public slots:
protected:
    explicit ConfigPresenter(QObject *parent = nullptr);
    ~ConfigPresenter() override;
private slots:
    void onDConfigChanged(const QString &cfg, const QString &key);

private:
    OrganizerConfig *conf = nullptr;
    QString confVersion;
    bool enable = false;
    bool enableVisibility = true;
    OrganizerMode curMode = OrganizerMode::kNormalized;
    Classifier curClassifier = Classifier::kType;
};

}

#define CfgPresenter ConfigPresenter::instance()
#endif   // CONFIGPRESENTER_H
