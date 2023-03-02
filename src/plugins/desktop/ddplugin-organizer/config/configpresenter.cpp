// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configpresenter.h"
#include "organizerconfig.h"
#include "mode/canvasorganizer.h"
#include "mode/normalized/fileclassifier.h"

#include <QDebug>
#include <QApplication>

using namespace ddplugin_organizer;

class ConfigPresenterGlobal : public ConfigPresenter{};
Q_GLOBAL_STATIC(ConfigPresenterGlobal, configPresenter)

ConfigPresenter::ConfigPresenter(QObject *parent) : QObject(parent)
{
    // created in main thread
    Q_ASSERT(qApp->thread() == thread());
}

ConfigPresenter::~ConfigPresenter()
{
    delete conf;
    conf = nullptr;
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
    enable = conf->isEnable();

    {
        int m = conf->mode();
        if (m < OrganizerMode::kNormalized || m > OrganizerMode::kCustom) {
            qWarning() << "mode is invalid:" << m;
            //m = 0;
        }
        // jsut release normal mode
        curMode = OrganizerMode::kNormalized; //static_cast<OrganizerMode>(m);
    }

    {
        int cf = conf->classification();
        if (cf < Classifier::kType || cf > Classifier::kSize) {
            qWarning() << "classification is invalid:" << cf;
            //cf = 0;
        }
        // // jsut release that classified by type
        curClassifier = Classifier::kType; //static_cast<Classifier>(cf);
    }

    return true;
}

void ConfigPresenter::setEnable(bool e)
{
    enable = e;
    conf->setEnable(e);
    conf->sync();
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
        return ;

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

