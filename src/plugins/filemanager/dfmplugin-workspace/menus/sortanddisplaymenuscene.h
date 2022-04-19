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
#ifndef SORTANDDISPLAYMENUSCENE_H
#define SORTANDDISPLAYMENUSCENE_H

#include "dfmplugin_workspace_global.h"

#include <interfaces/abstractmenuscene.h>
#include <interfaces/abstractscenecreator.h>

DPWORKSPACE_BEGIN_NAMESPACE
class SortAndDisplayMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "SortAndDisplayMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class SortAndDisplayMenuScenePrivate;
class SortAndDisplayMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
public:
    explicit SortAndDisplayMenuScene(QObject *parent = nullptr);
    virtual ~SortAndDisplayMenuScene() override;

    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    AbstractMenuScene *scene(QAction *action) const override;
    bool create(QMenu *parent) override;
    void updateState(QMenu *parent) override;
    bool triggered(QAction *action) override;

private:
    QScopedPointer<SortAndDisplayMenuScenePrivate> d;
};
DPWORKSPACE_END_NAMESPACE
#endif   // SORTANDDISPLAYMENUSCENE_H
