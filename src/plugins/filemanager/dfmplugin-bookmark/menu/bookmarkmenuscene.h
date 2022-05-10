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
#ifndef BOOKMARKMENUSCENE_H
#define BOOKMARKMENUSCENE_H

#include "dfmplugin_bookmark_global.h"

#include "dfm-base/interfaces/abstractmenuscene.h"
#include "dfm-base/interfaces/abstractscenecreator.h"

DPBOOKMARK_BEGIN_NAMESPACE

class BookmarkMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "BookmarkMenu";
    }

    virtual DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class BookmarkMenuScenePrivate;
class BookmarkMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
public:
    explicit BookmarkMenuScene(QObject *parent = nullptr);
    virtual ~BookmarkMenuScene() override;

    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual DFMBASE_NAMESPACE::AbstractMenuScene *scene(QAction *action) const override;

private:
    QScopedPointer<BookmarkMenuScenePrivate> d;
};

DPBOOKMARK_END_NAMESPACE

#endif   // BOOKMARKMENUSCENE_H
