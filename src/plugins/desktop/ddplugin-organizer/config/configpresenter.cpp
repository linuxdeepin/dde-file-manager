// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configpresenter.h"
#include "organizerconfig.h"
#include "mode/canvasorganizer.h"
#include "mode/normalized/fileclassifier.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QDebug>
#include <QApplication>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_organizer;

class ConfigPresenterGlobal : public ConfigPresenter
{
};
Q_GLOBAL_STATIC(ConfigPresenterGlobal, configPresenter)

static constexpr char kConfName[] { "org.deepin.dde.file-manager.desktop.organizer" };
static constexpr char kIsEnable[] { "enableOrganizer" };
static inline constexpr char kEnableVisible[] { "enableVisibility" };
static inline constexpr char kHideAllKeySeq[] { "hideAllKeySeq" };
static inline constexpr char kHideAllDialogRepeatNoMore[] { "hideAllDialogRepeatNoMore" };
static inline constexpr char kOrganizeAction[] { "organizeAction" };
static inline constexpr char kOrganizeEnabledCategories[] { "organizeCategories" };
static inline constexpr char kOrganizeMovingOptimize[] { "collectionMovingOptimize" };

static inline constexpr char kCatTypeApp[] { "kApp" };
static inline constexpr char kCatTypeDocument[] { "kDocument" };
static inline constexpr char kCatTypePicture[] { "kPicture" };
static inline constexpr char kCatTypeVideo[] { "kVideo" };
static inline constexpr char kCatTypeMusic[] { "kMusic" };
static inline constexpr char kCatTypeFolder[] { "kFolder" };
static inline constexpr char kCatTypeOther[] { "kOther" };

ConfigPresenter::ConfigPresenter(QObject *parent)
    : QObject(parent)
{
    // created in main thread
    Q_ASSERT(qApp->thread() == thread());
}

ConfigPresenter::~ConfigPresenter()
{
    delete conf;
    conf = nullptr;
}

void ConfigPresenter::onDConfigChanged(const QString &cfg, const QString &key)
{
    if (cfg != kConfName)
        return;

    if (key == kIsEnable) {
        bool ok = false;
        int value = DConfigManager::instance()->value(cfg, key).toInt(&ok);
        if (!ok) {
            fmWarning() << "Failed to parse enable state value:" << value;
            return;
        }
        if (value < 0) {
            fmWarning() << "Invalid enable state value:" << value;
            return;
        }
        bool val = value == 0 ? false : true;
        if (val != enable) {
            fmInfo() << "Organizer enable state changed from" << enable << "to" << val;
            emit changeEnableState(val);
        }
    }

    if (key == kOrganizeAction && organizeAction() == kAlways) {
        fmInfo() << "Organize action changed to 'Always', triggering desktop reorganization";
        Q_EMIT reorganizeDesktop();
    }

    if (key == kOrganizeMovingOptimize) {
        bool optimize = optimizeMovingPerformance();
        fmInfo() << "Moving optimization setting changed to:" << optimize;
        Q_EMIT optimizeStateChanged(optimize);
    }
}

ConfigPresenter *ConfigPresenter::instance()
{
    return configPresenter;
}

bool ConfigPresenter::initialize()
{
    if (conf)
        return false;

    conf = new OrganizerConfig();
    {
        bool ok = false;
        int value = DConfigManager::instance()->value(kConfName, kIsEnable).toInt(&ok);
        if (ok && value >= 0) {
            enable = value == 0 ? false : true;
            fmDebug() << "Enable state loaded from DConfig:" << enable;
        } else {
            enable = conf->isEnable();
            fmDebug() << "Enable state loaded from config file:" << enable;
        }
    }

    {
        enableVisibility = DConfigManager::instance()->value(kConfName, kEnableVisible).toBool();
    }

    {
        int m = conf->mode();
        if (m < OrganizerMode::kNormalized || m > OrganizerMode::kCustom) {
            fmWarning() << "Invalid organizer mode:" << m << "using default normalized mode";
            //m = 0;
        }
        // jsut release normal mode
        curMode = OrganizerMode::kNormalized;   //static_cast<OrganizerMode>(m);
    }

    {
        int cf = conf->classification();
        if (cf < Classifier::kType || cf > Classifier::kSize) {
            fmWarning() << "Invalid classification:" << cf << "using default type classification";
            //cf = 0;
        }
        // // jsut release that classified by type
        curClassifier = Classifier::kType;   //static_cast<Classifier>(cf);
    }

    connect(DConfigManager::instance(), &DConfigManager::valueChanged, this, &ConfigPresenter::onDConfigChanged);

    return true;
}

void ConfigPresenter::setVersion(const QString &v)
{
    confVersion = v;
    conf->setVersion(v);
    conf->sync();
}

QList<QSize> ConfigPresenter::surfaceSizes()
{
    return conf->surfaceSizes();
}

void ConfigPresenter::setSurfaceInfo(const QList<QWidget *> surfaces)
{
    QMap<QString, QString> resolutions;
    QString keyTemp = QString("Screen_%1");
    for (int i = 0; i < surfaces.count(); ++i) {
        auto surface = surfaces.at(i);
        Q_ASSERT(surface);
        resolutions.insert(keyTemp.arg(i + 1), QString("%1:%2").arg(surface->width()).arg(surface->height()));
    }
    conf->setScreenInfo(resolutions);
    conf->sync();
}

void ConfigPresenter::setLastStyleConfigId(const QString &id)
{
    conf->setLastStyleConfigId(id);
    conf->sync();
}

QString ConfigPresenter::lastStyleConfigId() const
{
    return conf->lastStyleConfigId();
}

bool ConfigPresenter::hasConfigId(const QString &configId) const
{
    return conf->hasConfigId(configId);
}

void ConfigPresenter::setEnable(bool e)
{
    enable = e;

    DConfigManager::instance()->setValue(kConfName, kIsEnable, e ? 1 : 0);
    conf->setEnable(e);
    conf->sync();
}

void ConfigPresenter::setEnableVisibility(bool v)
{
    enableVisibility = v;

    DConfigManager::instance()->setValue(kConfName, kEnableVisible, v);
}

void ConfigPresenter::setMode(OrganizerMode m)
{
    curMode = m;
    conf->setMode(m);
    conf->sync();
}

void ConfigPresenter::setClassification(Classifier cf)
{
    curClassifier = cf;
    conf->setClassification(cf);
    conf->sync();
}

QKeySequence ConfigPresenter::hideAllKeySequence() const
{
    return QKeySequence::fromString(DConfigManager::instance()->value(kConfName, kHideAllKeySeq, "Meta+O").toString());
}

void ConfigPresenter::setHideAllKeySequence(const QKeySequence &seq)
{
    DConfigManager::instance()->setValue(kConfName, kHideAllKeySeq, seq.toString());
}

bool ConfigPresenter::isRepeatNoMore() const
{
    return DConfigManager::instance()->value(kConfName, kHideAllDialogRepeatNoMore, false).toBool();
}

void ConfigPresenter::setRepeatNoMore(bool e)
{
    DConfigManager::instance()->setValue(kConfName, kHideAllDialogRepeatNoMore, e);
}

QList<CollectionBaseDataPtr> ConfigPresenter::customProfile() const
{
    return conf->collectionBase(true);
}

void ConfigPresenter::saveCustomProfile(const QList<CollectionBaseDataPtr> &baseDatas)
{
    conf->writeCollectionBase(true, baseDatas);
    conf->sync();
}

QList<CollectionBaseDataPtr> ConfigPresenter::normalProfile() const
{
    return conf->collectionBase(false);
}

void ConfigPresenter::saveNormalProfile(const QList<CollectionBaseDataPtr> &baseDatas)
{
    conf->writeCollectionBase(false, baseDatas);
    conf->sync();
}

CollectionStyle ConfigPresenter::customStyle(const QString &key) const
{
    return {};
    // if (key.isEmpty()) {
    //     fmWarning() << "Empty key provided for custom style lookup";
    //     return CollectionStyle();
    // }

    // return conf->collectionStyle(true, key);
}

void ConfigPresenter::updateCustomStyle(const CollectionStyle &style) const
{
    // if (style.key.isEmpty()) {
    //     fmWarning() << "Empty key in custom style, update ignored";
    //     return;
    // }

    // conf->updateCollectionStyle(true, style);
    // conf->sync();
}

void ConfigPresenter::writeCustomStyle(const QList<CollectionStyle> &styles) const
{
    // conf->writeCollectionStyle(true, styles);
    // conf->sync();
}

ItemCategories ConfigPresenter::enabledTypeCategories() const
{
    QStringList categories = DConfigManager::instance()->value(kConfName, kOrganizeEnabledCategories, "").toStringList();

    ItemCategories rets = kCatNone;
    rets |= categories.contains(kCatTypeApp) ? kCatApplication : kCatNone;
    rets |= categories.contains(kCatTypeDocument) ? kCatDocument : kCatNone;
    rets |= categories.contains(kCatTypePicture) ? kCatPicture : kCatNone;
    rets |= categories.contains(kCatTypeVideo) ? kCatVideo : kCatNone;
    rets |= categories.contains(kCatTypeMusic) ? kCatMusic : kCatNone;
    rets |= categories.contains(kCatTypeFolder) ? kCatFolder : kCatNone;
    rets |= categories.contains(kCatTypeOther) ? kCatOther : kCatNone;
    return rets;
}

void ConfigPresenter::setEnabledTypeCategories(ItemCategories flags)
{
    QStringList cats;
    if (flags & kCatApplication) cats.append(kCatTypeApp);
    if (flags & kCatDocument) cats.append(kCatTypeDocument);
    if (flags & kCatPicture) cats.append(kCatTypePicture);
    if (flags & kCatVideo) cats.append(kCatTypeVideo);
    if (flags & kCatMusic) cats.append(kCatTypeMusic);
    if (flags & kCatFolder) cats.append(kCatTypeFolder);
    if (flags & kCatOther) cats.append(kCatTypeOther);
    DConfigManager::instance()->setValue(kConfName, kOrganizeEnabledCategories, cats);
}

OrganizeAction ConfigPresenter::organizeAction() const
{
    int val = DConfigManager::instance()->value(kConfName, kOrganizeAction, 0).toInt();
    return val == 0 ? kOnTrigger : kAlways;
}

bool ConfigPresenter::organizeOnTriggered() const
{
    return organizeAction() == kOnTrigger;
}

bool ConfigPresenter::optimizeMovingPerformance() const
{
    bool val = DConfigManager::instance()->value(kConfName, kOrganizeMovingOptimize, false).toBool();
    return val;
}

CollectionStyle ConfigPresenter::normalStyle(const QString &configId, const QString &key) const
{
    if (key.isEmpty())
        return CollectionStyle();

    return conf->collectionStyle(configId, key);
}

void ConfigPresenter::updateNormalStyle(const QString &configId, const CollectionStyle &style) const
{
    if (style.key.isEmpty())
        return;

    conf->updateCollectionStyle(configId, style);
    conf->sync();
}

void ConfigPresenter::writeNormalStyle(const QString &configId, const QList<CollectionStyle> &styles) const
{
    conf->writeCollectionStyle(configId, styles);
    conf->sync();
}
