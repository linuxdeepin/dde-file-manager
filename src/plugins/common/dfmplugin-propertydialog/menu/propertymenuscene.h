/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#ifndef OPENWITHMENUSCENE_H
#define OPENWITHMENUSCENE_H

#include "dfmplugin_propertydialog_global.h"

#include <interfaces/abstractmenuscene.h>
#include <interfaces/abstractscenecreator.h>

DPPROPERTYDIALOG_BEGIN_NAMESPACE

class PropertyMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "Property";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class PropertyMenuScenePrivate;
class PropertyMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
public:
    explicit PropertyMenuScene(QObject *parent = nullptr);
    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    AbstractMenuScene *scene(QAction *action) const override;
    bool create(QMenu *parent) override;
    void updateState(QMenu *parent) override;
    bool triggered(QAction *action) override;

private:
    PropertyMenuScenePrivate *const d = nullptr;
};

DPPROPERTYDIALOG_END_NAMESPACE

#endif   // OPENWITHMENUSCENE_H
