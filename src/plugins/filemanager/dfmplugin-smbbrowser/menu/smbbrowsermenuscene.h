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
#ifndef SMBBROWSERMENUSCENE_H
#define SMBBROWSERMENUSCENE_H

#include "dfmplugin_smbbrowser_global.h"

#include <interfaces/abstractmenuscene.h>
#include <interfaces/abstractscenecreator.h>

DPSMBBROWSER_BEGIN_NAMESPACE

class SmbBrowserMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "SmbBrowserMenu";
    }

    virtual DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class SmbBrowserMenuScenePrivate;
class SmbBrowserMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
public:
    explicit SmbBrowserMenuScene(QObject *parent = nullptr);
    virtual ~SmbBrowserMenuScene() override;

    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual DFMBASE_NAMESPACE::AbstractMenuScene *scene(QAction *action) const override;

private:
    QScopedPointer<SmbBrowserMenuScenePrivate> d;
};

DPSMBBROWSER_END_NAMESPACE
#endif   // SMBBROWSERMENUSCENE_H
