/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: lanxuesong<lanxuesong@uniontech.com>
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

#include "basesortmenuscene.h"
#include "private/basesortmenuscene_p.h"

#include "menuscene/action_defines.h"

#include "dfm-base/dfm_menu_defines.h"

#include "plugins/common/dfmplugin-tag/dfmplugin_tag_global.h"
#include "plugins/filemanager/core/dfmplugin-trash/dfmplugin_trash_global.h"
#include "plugins/filemanager/core/dfmplugin-workspace/menus/workspacemenu_defines.h"
#include "plugins/desktop/core/ddplugin-canvas/menu/canvasmenu_defines.h"
#include "plugins/filemanager/core/dfmplugin-recent/dfmplugin_recent_global.h"
#include "plugins/common/dfmplugin-burn/dfmplugin_burn_global.h"
#include "plugins/common/dfmplugin-dirshare/dfmplugin_dirshare_global.h"
#include "plugins/common/dfmplugin-bookmark/dfmplugin_bookmark_global.h"
#include "plugins/common/core/dfmplugin-propertydialog/dfmplugin_propertydialog_global.h"

#include <QMenu>
#include <QDebug>

#include <mutex>

DPMENU_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

AbstractMenuScene *BaseSortMenuCreator::create()
{
    return new BaseSortMenuScene();
}

BaseSortMenuScenePrivate::BaseSortMenuScenePrivate(BaseSortMenuScene *qq)
    : AbstractMenuScenePrivate(qq), q(qq)
{
}

void BaseSortMenuScenePrivate::sort(QMenu *menu, const QList<QStringList> &rule)
{
    QList<QAction *> actions = menu->actions();
    QMap<QString, QAction *> actionsMap;

    for (QAction *action : actions) {
        menu->removeAction(action);
        insertActToMap(action, actionsMap);
    }

    QList<QAction *> actsNew;
    for (const QStringList &list : rule) {
        if (list.size() == 1) {
            const QString &id = list.first();
            if (actionsMap.count(id) > 0) {
                actsNew << actionsMap.value(id);
                actionsMap.remove(id);
            }
            if (id == dfmplugin_menu::ActionID::kSeparator && !actsNew.isEmpty() && actsNew.last()->property(DFMBASE_NAMESPACE::ActionPropertyKey::kActionID).toString() != dfmplugin_menu::ActionID::kSeparator) {
                QAction *separatorAct = new QAction(menu);
                separatorAct->setProperty(DFMBASE_NAMESPACE::ActionPropertyKey::kActionID, dfmplugin_menu::ActionID::kSeparator);
                separatorAct->setSeparator(true);
                actsNew << separatorAct;
            }
        } else if (list.size() > 1) {
            QAction *actionParent = nullptr;
            QMenu *menuParent = nullptr;
            QString parentId = list.first();

            for (int i = 0, nEnd = list.size(); i < nEnd; ++i) {
                const QString &id = list.at(i);
                if (actionsMap.count(id) > 0) {
                    if (0 == i) {
                        actionParent = actionsMap.value(id);
                        actionsMap.remove(id);
                    } else {
                        // check parent action
                        if (nullptr == actionParent) {
                            actionParent = new QAction(menu);
                            actionParent->setText(tr("%1").arg(predicateName.value(parentId)));
                            predicateAction[parentId] = actionParent;
                            actionParent->setProperty(DFMBASE_NAMESPACE::ActionPropertyKey::kActionID, parentId);
                        }
                        if (nullptr == menuParent) {
                            menuParent = new QMenu(menu);
                            actionParent->setMenu(menuParent);
                        }
                        menuParent->addAction(actionsMap.value(id));
                        actionsMap.remove(id);
                    }
                }
                if (checkPrefixRule(id)) {
                    auto list = findActionByKey(actionsMap, id);
                    if (!list.isEmpty()) {
                        if (nullptr == actionParent) {
                            actionParent = new QAction(tr("%1").arg(predicateName.value(parentId)), menu);
                            predicateAction[parentId] = actionParent;
                            actionParent->setProperty(DFMBASE_NAMESPACE::ActionPropertyKey::kActionID, parentId);
                        }
                        if (nullptr == menuParent) {
                            menuParent = new QMenu(menu);
                            actionParent->setMenu(menuParent);
                        }
                        for (QAction *action : list) {
                            menuParent->addAction(action);
                            actionsMap.remove(action->property(DFMBASE_NAMESPACE::ActionPropertyKey::kActionID).toString());
                        }
                    }
                }
            }
            if (actionParent && actionParent->menu())
                actsNew << actionParent;
        }
    }

    for (QAction *action : actionsMap.values())
        actsNew << action;

    menu->addActions(actsNew);
}

QList<QStringList> BaseSortMenuScenePrivate::actionSortRule()
{
    static QList<QStringList> ret;
    static std::once_flag flag;

    std::call_once(flag, [&]() {
        // file
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kOpen });   // 打开
        ret.append(QStringList { DPRECENT_NAMESPACE::RecentActionID::kOpenFileLocation });   // 打开文件所在位置
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kOpenWith,
                                 DPMENU_NAMESPACE::ActionID::kOpenWithApp,
                                 DPMENU_NAMESPACE::ActionID::kOpenWithCustom });   // 打开方式
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kOpenInNewWindow });   // 在新窗口打开
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kOpenInNewTab });   // 在新标签中打开

        // empty
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kNewFolder });
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kNewDoc,
                                 DPMENU_NAMESPACE::ActionID::kNewOfficeText,
                                 DPMENU_NAMESPACE::ActionID::kNewSpreadsheets,
                                 DPMENU_NAMESPACE::ActionID::kNewPresentation,
                                 DPMENU_NAMESPACE::ActionID::kNewPlainText });
        // empty trash
        ret.append(QStringList { DPTRASH_NAMESPACE::TrashActionId::kRestoreAll });
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kEmptyTrash });

        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kSeparator });

        ret.append(QStringList { dfmplugin_workspace::ActionID::kDisplayAs,
                                 dfmplugin_workspace::ActionID::kDisplayIcon,
                                 dfmplugin_workspace::ActionID::kDisplayList });   // 显示方式

        ret.append(QStringList { dfmplugin_workspace::ActionID::kSortBy,
                                 dfmplugin_workspace::ActionID::kSrtName,
                                 DPRECENT_NAMESPACE::RecentActionID::kSortByPath,
                                 DPTRASH_NAMESPACE::TrashActionId::kSourcePath,
                                 DPRECENT_NAMESPACE::RecentActionID::kSortByLastRead,
                                 dfmplugin_workspace::ActionID::kSrtTimeModified,
                                 DPTRASH_NAMESPACE::TrashActionId::kTimeDeleted,
                                 dfmplugin_workspace::ActionID::kSrtSize,
                                 dfmplugin_workspace::ActionID::kSrtType });   // 排序方式

        ret.append(QStringList { ddplugin_canvas::ActionID::kIconSize,
                                 ddplugin_canvas::ActionID::kIconSizeTiny,
                                 ddplugin_canvas::ActionID::kIconSizeSmall,
                                 ddplugin_canvas::ActionID::kIconSizeMedium,
                                 ddplugin_canvas::ActionID::kIconSizeLarge,
                                 ddplugin_canvas::ActionID::kIconSizeSuperLarge });   // icon size
        ret.append(QStringList { ddplugin_canvas::ActionID::kAutoArrange });
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kSeparator });

        QStringList listStageTo;
        listStageTo << DPBURN_NAMESPACE::ActionId::kStageKey;
        listStageTo << stageToRule();

        ret.append(listStageTo);   // 添加至光盘刻录

        ret.append(QStringList { DPBURN_NAMESPACE::ActionId::kMountImageKey });   // 挂载
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kSeparator });

        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kEmptyTrash });
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kSeparator });

        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kCut });   // 剪切
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kCopy });   // 复制
        ret.append(QStringList { DPRECENT_NAMESPACE::RecentActionID::kRemove });
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kRename });   // 重命名
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kDelete });   // 删除
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kPaste });
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kSelectAll });
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kSeparator });

        ret.append(QStringList { DPDIRSHARE_NAMESPACE::ShareActionId::kActAddShareKey });   // 添加共享
        ret.append(QStringList { DPDIRSHARE_NAMESPACE::ShareActionId::kActRemoveShareKey });   // 移除共享

        QStringList sendToList;
        sendToList << DPMENU_NAMESPACE::ActionID::kSendTo;
        sendToList << DPMENU_NAMESPACE::ActionID::kCreateSymlink;
        sendToList << DPMENU_NAMESPACE::ActionID::kSendToDesktop;
        sendToList << sendToRule();

        ret.append(sendToList);   // 发送到
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kShare,
                                 DPMENU_NAMESPACE::ActionID::kShareToBluetooth });   // 共享
        ret.append(QStringList { DPBOOKMARK_NAMESPACE::BookmarkActionId::kActAddBookmarkKey });   // 添加书签
        ret.append(QStringList { DPBOOKMARK_NAMESPACE::BookmarkActionId::kActRemoveBookmarkKey });   // 移除书签
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kSetAsWallpaper });   // 设置壁纸
        ret.append(QStringList { ddplugin_canvas::ActionID::kDisplaySettings });   // 壁纸设置
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kSeparator });

        ret.append(QStringList { DPTAG_NAMESPACE::TagActionId::kActTagAddKey });   // 标记信息
        ret.append(QStringList { DPTAG_NAMESPACE::TagActionId::kActTagColorListKey });   // 标记颜色列表
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kSeparator });

        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kOpenAsAdmin });
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kOpenInTerminal });
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kSeparator });

        ret.append(QStringList { ddplugin_canvas::ActionID::kRefresh });
        ret.append(QStringList { DPMENU_NAMESPACE::ActionID::kSeparator });

        ret.append(QStringList { DPPROPERTYDIALOG_NAMESPACE::PropertyActionId::kProperty });
    });

    return ret;
}

QStringList BaseSortMenuScenePrivate::sendToRule()
{
    static QStringList list;

    static std::once_flag flag;
    std::call_once(flag, []() {
        list << DPMENU_NAMESPACE::ActionID::kSendToRemovablePrefix;
        list << DPBURN_NAMESPACE::ActionId::kSendToOptical;
    });

    return list;
}

QStringList BaseSortMenuScenePrivate::stageToRule()
{
    static QStringList list;

    static std::once_flag flag;
    std::call_once(flag, []() {
        list << DPBURN_NAMESPACE::ActionId::kStagePrex;
    });

    return list;
}

bool BaseSortMenuScenePrivate::checkPrefixRule(const QString &id)
{
    static QStringList rules;

    static std::once_flag flag;
    std::call_once(flag, [&]() {
        rules << sendToRule();
        rules << stageToRule();
    });
    auto it = std::find_if(rules.begin(), rules.end(), [id](const QString &rule) {
        return id.startsWith(rule);
    });
    return it != rules.end();
}

QList<QAction *> BaseSortMenuScenePrivate::findActionByKey(const QMap<QString, QAction *> &map, const QString &key)
{
    QList<QAction *> list;
    auto it = map.constBegin();
    while (it != map.constEnd()) {
        if (it.key().startsWith(key))
            list.append(it.value());
        ++it;
    }
    return list;
}

void BaseSortMenuScenePrivate::insertActToMap(QAction *action, QMap<QString, QAction *> &map)
{
    if (action->property(DFMBASE_NAMESPACE::ActionPropertyKey::kActionID).toString() == dfmplugin_menu::ActionID::kSeparator)
        return;
    if (!action->menu()) {
        map.insert(action->property(DFMBASE_NAMESPACE::ActionPropertyKey::kActionID).toString(), action);
    } else {
        QList<QAction *> acts = action->menu()->actions();
        action->setMenu(nullptr);
        map.insert(action->property(DFMBASE_NAMESPACE::ActionPropertyKey::kActionID).toString(), action);
        for (QAction *action : acts) {
            insertActToMap(action, map);
        }
    }
}

BaseSortMenuScene::BaseSortMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new BaseSortMenuScenePrivate(this))
{
    d->predicateName[ActionID::kSendTo] = tr("Send to");   // virtual action, when has secend action, need generate this action
}

BaseSortMenuScene::~BaseSortMenuScene()
{
}

QString BaseSortMenuScene::name() const
{
    return BaseSortMenuCreator::name();
}

bool BaseSortMenuScene::initialize(const QVariantHash &params)
{
    return AbstractMenuScene::initialize(params);
}

bool BaseSortMenuScene::create(QMenu *parent)
{
    return AbstractMenuScene::create(parent);
}

void BaseSortMenuScene::updateState(QMenu *parent)
{
    d->sort(parent, d->actionSortRule());
    AbstractMenuScene::updateState(parent);
}

bool BaseSortMenuScene::triggered(QAction *action)
{
    return AbstractMenuScene::triggered(action);
}

AbstractMenuScene *BaseSortMenuScene::scene(QAction *action) const
{
    return AbstractMenuScene::scene(action);
}
