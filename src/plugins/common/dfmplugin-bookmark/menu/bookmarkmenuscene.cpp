// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmplugin_bookmark_global.h"
#include "bookmarkmenuscene.h"
#include "private/bookmarkmenuscene_p.h"
#include "controller/bookmarkmanager.h"

#include "plugins/common/dfmplugin-menu/menuscene/action_defines.h"
#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_menu_defines.h>

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
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    if (d->isEmptyArea)
        return false;

    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();

    if (d->selectFiles.count() <= 0)
        return AbstractMenuScene::initialize(params);

    d->showBookMarkMenu = true;
    const auto &tmpParams = dfmplugin_menu_util::menuPerfectParams(params);
    d->isSystemPathIncluded = tmpParams.value(MenuParamKey::kIsSystemPathIncluded, false).toBool();
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->focusFile = d->selectFiles.first();

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

    d->focusFileInfo = InfoFactory::create<FileInfo>(d->focusFile);
    if ((d->focusFileInfo && !d->focusFileInfo->isAttributes(OptInfoType::kIsDir)) || d->isSystemPathIncluded)
        return AbstractMenuScene::create(parent);

    auto addAct = [=](const QString &key) {
        auto action = parent->addAction(d->predicateName[key]);
        action->setText(d->predicateName.value(key));
        action->setProperty(ActionPropertyKey::kActionID, key);
        d->predicateAction.insert(key, action);
    };

    if (!BookMarkManager::instance()->getBookMarkDataMap().contains(d->focusFile))
        addAct(BookmarkActionId::kActAddBookmarkKey);
    else
        addAct(BookmarkActionId::kActRemoveBookmarkKey);

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
        std::for_each(d->selectFiles.cbegin(), d->selectFiles.cend(), [](const QUrl &item) {
            BookMarkManager::instance()->removeBookMark(item);
        });
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
