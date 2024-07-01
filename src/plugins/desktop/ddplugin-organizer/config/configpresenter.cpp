// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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
        if (!ok)
            return;
        if (value < 0)
            return;
        bool val = value == 0 ? false : true;
        if (val != enable)
            emit changeEnableState(val);
    }

    if (key == kOrganizeAction && organizeAction() == kAlways) {
        Q_EMIT reorganizeDesktop();
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
        } else {
            enable = conf->isEnable();
        }
    }

    {
        enableVisibility = DConfigManager::instance()->value(kConfName, kEnableVisible).toBool();
    }

    {
        int m = conf->mode();
        if (m < OrganizerMode::kNormalized || m > OrganizerMode::kCustom) {
            fmWarning() << "mode is invalid:" << m;
            //m = 0;
        }
        // jsut release normal mode
        curMode = OrganizerMode::kNormalized;   //static_cast<OrganizerMode>(m);
    }

    {
        int cf = conf->classification();
        if (cf < Classifier::kType || cf > Classifier::kSize) {
            fmWarning() << "classification is invalid:" << cf;
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
    if (key.isEmpty())
        return CollectionStyle();

    return conf->collectionStyle(true, key);
}

void ConfigPresenter::updateCustomStyle(const CollectionStyle &style) const
{
    if (style.key.isEmpty())
        return;

    conf->updateCollectionStyle(true, style);
    conf->sync();
}

void ConfigPresenter::writeCustomStyle(const QList<CollectionStyle> &styles) const
{
    conf->writeCollectionStyle(true, styles);
    conf->sync();
}

ItemCategories ConfigPresenter::enabledTypeCategories() const
{
    return ItemCategories(conf->enabledTypeCategories());
}

void ConfigPresenter::setEnabledTypeCategories(ItemCategories flags)
{
    conf->setEnabledTypeCategories(flags);
    conf->sync();
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

CollectionStyle ConfigPresenter::normalStyle(const QString &key) const
{
    if (key.isEmpty())
        return CollectionStyle();

    return conf->collectionStyle(false, key);
}

void ConfigPresenter::updateNormalStyle(const CollectionStyle &style) const
{
    if (style.key.isEmpty())
        return;

    conf->updateCollectionStyle(false, style);
    conf->sync();
}

void ConfigPresenter::writeNormalStyle(const QList<CollectionStyle> &styles) const
{
    conf->writeCollectionStyle(false, styles);
    conf->sync();
}
