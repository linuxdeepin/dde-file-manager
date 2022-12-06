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
#include "dfmplugin_bookmark_global.h"
#include "bookmarkmenuscene.h"
#include "private/bookmarkmenuscene_p.h"
#include "controller/bookmarkmanager.h"

#include "plugins/common/core/dfmplugin-menu/menuscene/action_defines.h"
#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_menu_defines.h"

#include <dfm-framework/dpf.h>

#include <QMenu>
#include <QMap>

using namespace dfmplugin_bookmark;
DFMBASE_USE_NAMESPACE

BookmarkMenuScene::BookmarkMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new BookmarkMenuScenePrivate(this))
{
}

BookmarkMenuScene::~BookmarkMenuScene()
{
}

QString BookmarkMenuScene::name() const
{
    return BookmarkMenuCreator::name();
}

bool BookmarkMenuScene::initialize(const QVariantHash &params)
{
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();

    if (d->selectFiles.count() <= 0)
        return AbstractMenuScene::initialize(params);

    d->showBookMarkMenu = true;
    const auto &tmpParams = dfmplugin_menu_util::menuPerfectParams(params);
    d->isSystemPathIncluded = tmpParams.value(MenuParamKey::kIsSystemPathIncluded, false).toBool();

    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->focusFile = d->selectFiles.first();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    d->predicateName.insert(BookmarkActionId::kActAddBookmarkKey, tr("Pin to quick access"));
    d->predicateName.insert(BookmarkActionId::kActRemoveBookmarkKey, tr("Remove from quick access"));

    return AbstractMenuScene::initialize(params);
}

bool BookmarkMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    if (!d->showBookMarkMenu)
        return AbstractMenuScene::create(parent);

    for (const auto &file : d->selectFiles) {
        auto info = InfoFactory::create<AbstractFileInfo>(file);
        if ((info && !info->isAttributes(AbstractFileInfo::FileIsType::kIsDir)) || d->isSystemPathIncluded)
            return AbstractMenuScene::create(parent);
    }

    auto action = parent->addAction(d->predicateName[BookmarkActionId::kActAddBookmarkKey]);
    action->setText(d->predicateName.value(BookmarkActionId::kActAddBookmarkKey));
    action->setProperty(ActionPropertyKey::kActionID, QString(BookmarkActionId::kActAddBookmarkKey));
    d->predicateAction.insert(BookmarkActionId::kActAddBookmarkKey, action);

    return AbstractMenuScene::create(parent);
}

void BookmarkMenuScene::updateState(QMenu *parent)
{
    AbstractMenuScene::updateState(parent);
}

bool BookmarkMenuScene::triggered(QAction *action)
{
    if (!d->focusFile.isValid())
        return AbstractMenuScene::triggered(action);

    if (action == d->predicateAction.value(BookmarkActionId::kActAddBookmarkKey)) {
        BookMarkManager::instance()->addBookMark(d->selectFiles);
    } else if (action == d->predicateAction.value(BookmarkActionId::kActRemoveBookmarkKey)) {
        BookMarkManager::instance()->removeBookMark(d->focusFile);
    }

    return AbstractMenuScene::triggered(action);
}

AbstractMenuScene *BookmarkMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<BookmarkMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

AbstractMenuScene *BookmarkMenuCreator::create()
{
    return new BookmarkMenuScene();
}
