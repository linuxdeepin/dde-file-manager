/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef COMPUTERMENUSCENE_H
#define COMPUTERMENUSCENE_H

#include "dfmplugin_computer_global.h"
#include "utils/computerutils.h"

#include "dfm-base/interfaces/abstractmenuscene.h"
#include "dfm-base/interfaces/abstractscenecreator.h"

DPCOMPUTER_BEGIN_NAMESPACE

class ComputerMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    inline static QString name()
    {
        return ComputerUtils::menuSceneName();
    }

    virtual DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class ComputerMenuScenePrivate;
class ComputerMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
public:
    explicit ComputerMenuScene(QObject *parent = nullptr);
    virtual ~ComputerMenuScene() override;

    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual dfmbase::AbstractMenuScene *scene(QAction *action) const override;

private:
    QScopedPointer<ComputerMenuScenePrivate> d;
};

DPCOMPUTER_END_NAMESPACE

#endif   // COMPUTERMENUSCENE_H
