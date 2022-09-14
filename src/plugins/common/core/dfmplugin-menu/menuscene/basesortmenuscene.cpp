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

#include <QMenu>

#include <mutex>

DPMENU_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

namespace ActionID {
static constexpr char kActionID[] { "actionID" };
static constexpr char kSeparatorLine[] { "separator-line" };
static constexpr char kSendTo[] { "send-to" };
}

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
            if (id == ActionID::kSeparatorLine && !actsNew.isEmpty() && actsNew.last()->property(ActionID::kActionID).toString() != ActionID::kSeparatorLine) {
                QAction *separatorAct = new QAction(menu);
                separatorAct->setProperty(ActionID::kActionID, ActionID::kSeparatorLine);
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
                            actionParent->setProperty(ActionID::kActionID, parentId);
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
                            actionParent->setProperty(ActionID::kActionID, parentId);
                        }
                        if (nullptr == menuParent) {
                            menuParent = new QMenu(menu);
                            actionParent->setMenu(menuParent);
                        }
                        for (QAction *action : list) {
                            menuParent->addAction(action);
                            actionsMap.remove(action->property(ActionID::kActionID).toString());
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
        ret.append(QStringList { "open" });   // 打开
        ret.append(QStringList { "open-file-location" });   // 打开文件所在位置
        ret.append(QStringList { "open-with",   // 打开方式
                                 "open-with-app",
                                 "open-with-custom" });
        ret.append(QStringList { "open-in-new-window" });   // 在新窗口打开
        ret.append(QStringList { "open-in-new-tab" });   // 在新标签中打开

        // empty
        ret.append(QStringList { "new-folder" });
        ret.append(QStringList { "new-document",
                                 "new-office-text",
                                 "new-spreadsheets",
                                 "new-presentation",
                                 "new-plain-text" });
        // empty trash
        ret.append(QStringList { "restore-all" });
        ret.append(QStringList { "empty-trash" });

        ret.append(QStringList { ActionID::kSeparatorLine });

        ret.append(QStringList { "auto-arrange" });
        ret.append(QStringList { "display-as",   // 显示方式
                                 "display-as-icon",
                                 "display-as-list" });

        ret.append(QStringList { "sort-by",
                                 "sort-by-name",
                                 "sort-by-path",
                                 "sort-by-source-path",
                                 "sort-by-lastRead",
                                 "sort-by-time-modified",
                                 "sort-by-time-deleted",
                                 "sort-by-size",
                                 "sort-by-type" });   // 排序方式

        ret.append(QStringList { "icon-size",
                                 "tiny",
                                 "small",
                                 "medium",
                                 "large",
                                 "super-large" });   // icon size
        ret.append(QStringList { ActionID::kSeparatorLine });

        QStringList listStageTo;
        listStageTo << "stage-file-to-burning";
        listStageTo << stageToRule();

        ret.append(listStageTo);   // 添加至光盘刻录

        ret.append(QStringList { "mount-image" });   // 挂载
        ret.append(QStringList { ActionID::kSeparatorLine });

        ret.append(QStringList { "empty-trash" });
        ret.append(QStringList { ActionID::kSeparatorLine });

        ret.append(QStringList { "cut" });   // 剪切
        ret.append(QStringList { "copy" });   // 复制
        ret.append(QStringList { "remove" });   // 移除
        ret.append(QStringList { "rename" });   // 重命名
        ret.append(QStringList { "delete" });   // 删除
        ret.append(QStringList { "paste" });
        ret.append(QStringList { "select-all" });
        ret.append(QStringList { ActionID::kSeparatorLine });

        ret.append(QStringList { "refresh" });
        ret.append(QStringList { ActionID::kSeparatorLine });

        ret.append(QStringList { "add-share" });   // 添加共享
        ret.append(QStringList { "remove-share" });   // 移除共享

        QStringList sendToList;
        sendToList << ActionID::kSendTo;
        sendToList << "create-system-link";
        sendToList << "send-to-desktop";
        sendToList << sendToRule();

        ret.append(sendToList);   // 发送到
        ret.append(QStringList { "share",
                                 "share-to-bluetooth" });   // 共享
        ret.append(QStringList { "add-bookmark" });   // 添加书签
        ret.append(QStringList { "remove-bookmark" });   // 移除书签
        ret.append(QStringList { "set-as-wallpaper" });   // 设置壁纸
        ret.append(QStringList { "display-settings" });   // 显示设置
        ret.append(QStringList { "wallpaper-settings" });   // 壁纸与屏保
        ret.append(QStringList { ActionID::kSeparatorLine });

        ret.append(QStringList { "tag-add" });   // 标记信息
        ret.append(QStringList { "tag-color-list" });   // 标记颜色列表
        ret.append(QStringList { ActionID::kSeparatorLine });

        ret.append(QStringList { "open-as-administrator" });
        ret.append(QStringList { "open-in-terminal" });
        ret.append(QStringList { ActionID::kSeparatorLine });

        ret.append(QStringList { "property" });
    });

    return ret;
}

QStringList BaseSortMenuScenePrivate::sendToRule()
{
    static QStringList list;

    static std::once_flag flag;
    std::call_once(flag, []() {
        list << "send-to-removable-";
        list << "send-file-to-burnning-";
    });

    return list;
}

QStringList BaseSortMenuScenePrivate::stageToRule()
{
    static QStringList list;

    static std::once_flag flag;
    std::call_once(flag, []() {
        list << "_stage-file-to-burning-";
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
    if (action->property(ActionID::kActionID).toString() == ActionID::kSeparatorLine)
        return;
    if (!action->menu()) {
        map.insert(action->property(ActionID::kActionID).toString(), action);
    } else {
        QList<QAction *> acts = action->menu()->actions();
        action->setMenu(nullptr);
        map.insert(action->property(ActionID::kActionID).toString(), action);
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
