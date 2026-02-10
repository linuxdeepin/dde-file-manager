// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTICALMENUSCENE_H
#define OPTICALMENUSCENE_H

#include "dfmplugin_optical_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_optical {

class OpticalMenuScenePrivate;
class OpticalMenuSceneCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "OpticalMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class OpticalMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
public:
    explicit OpticalMenuScene(QObject *parent = nullptr);
    ~OpticalMenuScene() override;

    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    void updateState(QMenu *parent) override;

private:
    QScopedPointer<OpticalMenuScenePrivate> d;
};

}

#endif   // OPTICALMENUSCENE_H
