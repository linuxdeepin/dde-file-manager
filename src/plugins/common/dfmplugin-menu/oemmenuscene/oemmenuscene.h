/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef OEMMENUSCENE_H
#define OEMMENUSCENE_H

#include "dfmplugin_menu_global.h"

#include "interfaces/abstractmenuscene.h"
#include "interfaces/abstractscenecreator.h"

DPMENU_BEGIN_NAMESPACE

class OemMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "OemMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class OemMenuScenePrivate;
class OemMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
public:
    explicit OemMenuScene(QObject *parent = nullptr);
    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    AbstractMenuScene *scene(QAction *action) const override;
    bool create(QMenu *parent) override;
    void updateState(QMenu *parent) override;
    bool triggered(QAction *action) override;

private:
    OemMenuScenePrivate *const d = nullptr;
};

DPMENU_END_NAMESPACE
#endif   // OEMMENUSCENE_H
