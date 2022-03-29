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
#ifndef SHAREMENUSCENE_H
#define SHAREMENUSCENE_H

#include "dfmplugin_dirshare_global.h"

#include <interfaces/abstractmenuscene.h>
#include <interfaces/abstractscenecreator.h>

DPDIRSHARE_BEGIN_NAMESPACE

class ShareMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "ShareMenu";
    }

    virtual dfmbase::AbstractMenuScene *create() override;
};

class ShareMenuScenePrivate;
class ShareMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
public:
    explicit ShareMenuScene(QObject *parent = nullptr);
    virtual ~ShareMenuScene() override;

    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual DFMBASE_NAMESPACE::AbstractMenuScene *scene(QAction *action) const override;

private:
    QScopedPointer<ShareMenuScenePrivate> d;
};

DPDIRSHARE_END_NAMESPACE
#endif   // SHAREMENUSCENE_H
