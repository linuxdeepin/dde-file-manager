/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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
#ifndef TAGMENUSCENE_H
#define TAGMENUSCENE_H

#include "dfmplugin_tag_global.h"

#include "dfm-base/interfaces/abstractmenuscene.h"
#include "dfm-base/interfaces/abstractscenecreator.h"

DPTAG_BEGIN_NAMESPACE
class TagColorListWidget;
class TagMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "TagMenu";
    }

    virtual DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class TagMenuScenePrivate;
class TagMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
public:
    explicit TagMenuScene(QObject *parent = nullptr);
    virtual ~TagMenuScene() override;

    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual DFMBASE_NAMESPACE::AbstractMenuScene *scene(QAction *action) const override;

public slots:
    void onHoverChanged(const QColor &color);
    void onColorClicked(const QColor &color);

private:
    QScopedPointer<TagMenuScenePrivate> d;

    TagColorListWidget *getMenuListWidget() const;

    QAction *createTagAction() const;
    QAction *createColorListAction() const;
};

DPTAG_END_NAMESPACE

#endif   // TAGMENUSCENE_H
