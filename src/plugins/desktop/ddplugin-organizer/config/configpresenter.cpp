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
#include "configpresenter.h"
#include "organizerconfig.h"
#include "mode/canvasorganizer.h"
#include "mode/normalized/fileclassifier.h"

#include <QDebug>
#include <QApplication>

DDP_ORGANIZER_USE_NAMESPACE

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
            m = 0;
        }
        curMode = static_cast<OrganizerMode>(m);
    }

    {
        int cf = conf->classification();
        if (cf < Classifier::kType || cf > Classifier::kSize) {
            qWarning() << "classification is invalid:" << cf;
            cf = 0;
        }
        curClassifier = static_cast<Classifier>(cf);
    }
    return true;
}

bool ConfigPresenter::isEnable() const
{
    return enable;
}

void ConfigPresenter::setEnable(bool e)
{
    enable = e;
    conf->setEnable(e);
    conf->sync();
}

OrganizerMode ConfigPresenter::mode() const
{
    return curMode;
}

void ConfigPresenter::setMode(OrganizerMode m)
{
    curMode = m;
    conf->setMode(m);
    conf->sync();
}

Classifier ConfigPresenter::classification() const
{
    return curClassifier;
}

void ConfigPresenter::setClassification(Classifier cf)
{
    curClassifier = cf;
    conf->setClassification(cf);
    conf->sync();
}

