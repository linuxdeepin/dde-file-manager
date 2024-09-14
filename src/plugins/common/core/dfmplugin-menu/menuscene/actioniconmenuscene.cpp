// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "actioniconmenuscene.h"
#include "private/actioniconmenuscene_p.h"

#include <dfm-base/dfm_menu_defines.h>

#include <QMenu>
#include <QDebug>

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#    include <QGSettings>
#endif

DPMENU_USE_NAMESPACE

AbstractMenuScene *ActionIconMenuCreator::create()
{
    return new ActionIconMenuScene();
}

ActionIconMenuScenePrivate::ActionIconMenuScenePrivate(ActionIconMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
}

ActionIconMenuScene::ActionIconMenuScene(QObject *parent)
    : AbstractMenuScene(parent)
{
}

QString ActionIconMenuScene::name() const
{
    return ActionIconMenuCreator::name();
}

void ActionIconMenuScene::updateState(QMenu *parent)
{
    bool visible = actionIconVisible();
    fmDebug() << "menu: action icon visible: " << visible;
    if (!visible)
        return AbstractMenuScene::updateState(parent);

    static const QMap<QString, QIcon> actionIcons {
        { "new-folder", QIcon::fromTheme("folder-new") },
        { "new-document", QIcon::fromTheme("document-new") },
        { "open-in-new-window", QIcon::fromTheme("window-new") },
        { "computer-open-in-win", QIcon::fromTheme("window-new") },
        { "open-share-in-new-win", QIcon::fromTheme("window-new") },
        { "open-in-new-tab", QIcon::fromTheme("tab-new") },
        { "computer-open-in-tab", QIcon::fromTheme("tab-new") },
        { "open-share-in-new-tab", QIcon::fromTheme("tab-new") },
        { "open-in-terminal", QIcon::fromTheme("utilities-terminal") },
        { "add-bookmark", QIcon::fromTheme("bookmark-new") },
        { "remove-bookmark", QIcon::fromTheme("bookmark-remove") },
        { "copy", QIcon::fromTheme("edit-copy") },
        { "paste", QIcon::fromTheme("edit-paste") },
        { "cut", QIcon::fromTheme("edit-cut") },
        { "rename", QIcon::fromTheme("edit-rename") },
        { "computer-rename", QIcon::fromTheme("edit-rename") },
        { "delete", QIcon::fromTheme("edit-delete") },
        { "remove-share", QIcon::fromTheme("edit-delete-shred") },
        { "cancel-sharing", QIcon::fromTheme("edit-delete-shred") },
        { "add-share", QIcon::fromTheme("document-share") },
        { "select-all", QIcon::fromTheme("edit-select-all") },
        { "create-system-link", QIcon::fromTheme("insert-link") },
        { "property", QIcon::fromTheme("document-properties") },
        { "share-property", QIcon::fromTheme("document-properties") },
        { "computer-property", QIcon::fromTheme("document-properties") },
    };

    QList<QMenu *> menus { parent };
    do {
        auto menu = menus.takeFirst();
        auto actions = menu->actions();
        for (int i = actions.count() - 1; i >= 0; --i) {
            auto action = actions.at(i);
            const QString &id = action->property(ActionPropertyKey::kActionID).toString();
            if (!id.isEmpty() && actionIcons.contains(id))
                action->setIcon(actionIcons[id]);

            if (auto subMenu = action->menu())
                menus.append(subMenu);
        }
    } while (menus.count() > 0);

    AbstractMenuScene::updateState(parent);
}

bool ActionIconMenuScene::actionIconVisible() const
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    // the gsetting control for action icon visible
    if (QGSettings::isSchemaInstalled("com.deepin.dde.filemanager.general")) {
        const QGSettings setting("com.deepin.dde.filemanager.general",
                                 "/com/deepin/dde/filemanager/general/");

        if (setting.keys().contains("contextMenuIcons")) {
            auto visible = setting.get("contextMenuIcons");
            if (visible.isValid())
                return visible.toBool();
        }
    }
#endif

    return false;
}
