// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "basesortmenuscene.h"
#include "basesortmenuscene_p.h"

#include <QMenu>

#include <mutex>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

namespace ActionID {
static constexpr char kActionID[] { "actionID" };
static constexpr char kSeparatorLine[] { "separator-line" };
}

AbstractMenuScene *BaseSortMenuCreator::create()
{
    return new BaseSortMenuScene();
}

BaseSortMenuScenePrivate::BaseSortMenuScenePrivate(BaseSortMenuScene *qq)
    : AbstractMenuScenePrivate(qq), q(qq)
{
}

void BaseSortMenuScenePrivate::sortMenuActions(QMenu *menu, const QStringList &sortRule, bool isFuzzy)
{
    auto findIndex = [&isFuzzy, &sortRule](const QString &property) {
        int index1 = -1;
        for (int i = 0, nEnd = sortRule.size(); i < nEnd; ++i) {
            if (property == sortRule[i] || (isFuzzy && property.startsWith(sortRule[i]))) {
                index1 = i;
                break;
            }
        }
        return index1;
    };

    auto actions = menu->actions();
    // sort
    std::sort(actions.begin(), actions.end(), [findIndex](QAction *act1, QAction *act2) {
        const auto &property1 = act1->property(ActionID::kActionID).toString();
        auto index1 = findIndex(property1);
        if (index1 == -1)
            return false;

        const auto &property2 = act2->property(ActionID::kActionID).toString();
        auto index2 = findIndex(property2);
        if (index2 == -1)
            return true;

        return index1 < index2;
    });

    // insert separator func
    std::function<void(int)> insertSeparator;
    insertSeparator = [&](int index) {
        if (index >= sortRule.size() || sortRule[index] == ActionID::kSeparatorLine)
            return;

        auto rule = sortRule[index];
        auto iter = std::find_if(actions.begin(), actions.end(), [&rule, &isFuzzy](const QAction *act) {
            auto p = act->property(ActionID::kActionID);
            return p == rule || (isFuzzy && p.toString().startsWith(rule));
        });

        if (iter != actions.end()) {
            QAction *separatorAct = new QAction(menu);
            separatorAct->setProperty(ActionID::kActionID, ActionID::kSeparatorLine);
            separatorAct->setSeparator(true);
            actions.insert(iter, separatorAct);
        } else {
            insertSeparator(++index);
        }
    };

    // insert separator
    int index = sortRule.indexOf(ActionID::kSeparatorLine);
    while (index != -1) {
        if (++index >= sortRule.size())
            break;

        insertSeparator(index);
        index = sortRule.indexOf(ActionID::kSeparatorLine, index);
    }

    menu->addActions(actions);
}

void BaseSortMenuScenePrivate::sortPrimaryMenu(QMenu *menu)
{
    const QStringList &sortRule = primaryMenuRule();
    sortMenuActions(menu, sortRule, false);
}

void BaseSortMenuScenePrivate::sortSecondaryMenu(QMenu *menu)
{
    const QMap<QString, QStringList> &sortRuleMap = secondaryMenuRule();

    auto actions = menu->actions();
    for (QAction *action : actions) {
        QMenu *secondaryMenu = action->menu();
        if (secondaryMenu && !secondaryMenu->actions().isEmpty()) {
            const auto &key = action->property(ActionID::kActionID).toString();
            if (sortRuleMap.count(key) > 0) {
                const QStringList &sortRule = sortRuleMap.value(key);
                sortMenuActions(secondaryMenu, sortRule, true);
            }
        }
    }
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

QStringList BaseSortMenuScenePrivate::primaryMenuRule()
{
    static QStringList ret;
    static std::once_flag flag;

    std::call_once(flag, [&]() {
        // file
        ret.append("open");   // 打开
        ret.append("open-file-location");   // 打开文件所在位置
        ret.append("open-with");   // 打开方式
        ret.append("open-in-new-window");   // 在新窗口打开
        ret.append("open-in-new-tab");   // 在新标签中打开

        // empty
        ret.append("new-folder");
        ret.append("new-document");
        // empty trash
        ret.append("restore-all");
        ret.append("empty-trash");

        ret.append(ActionID::kSeparatorLine);

        ret.append("auto-arrange");
        ret.append("display-as");   // 显示方式

        ret.append("sort-by");   // 排序方式

        ret.append("icon-size");   // icon size
        ret.append("refresh");
        ret.append(ActionID::kSeparatorLine);

        ret.append("stage-file-to-burning");   // 添加至光盘刻录

        ret.append("mount-image");   // 挂载
        ret.append(ActionID::kSeparatorLine);

        ret.append("empty-trash");
        ret.append(ActionID::kSeparatorLine);

        ret.append("cut");   // 剪切
        ret.append("copy");   // 复制
        ret.append("remove");   // 移除
        ret.append("rename");   // 重命名
        ret.append("delete");   // 删除
        ret.append("paste");
        ret.append("reverse-select");
        ret.append("select-all");
        ret.append(ActionID::kSeparatorLine);

        ret.append("add-share");   // 添加共享
        ret.append("remove-share");   // 移除共享

        ret.append("send-to");   // 发送到
        ret.append("share");   // 共享
        ret.append("add-bookmark");   // 添加书签
        ret.append("remove-bookmark");   // 移除书签
        ret.append("set-as-wallpaper");   // 设置壁纸
        ret.append("display-settings");   // 显示设置
        ret.append("wallpaper-settings");   // 壁纸与屏保
        ret.append(ActionID::kSeparatorLine);

        ret.append("tag-add");   // 标记信息
        ret.append("tag-color-list");   // 标记颜色列表
        ret.append(ActionID::kSeparatorLine);

        ret.append("open-as-administrator");
        ret.append("open-in-terminal");
        ret.append(ActionID::kSeparatorLine);

        ret.append("property");
    });

    return ret;
}

QMap<QString, QStringList> BaseSortMenuScenePrivate::secondaryMenuRule()
{
    static QMap<QString, QStringList> ret;
    static std::once_flag flag;

    std::call_once(flag, [&]() {
        // file
        ret.insert("open-with",
                   QStringList { "open-with-app",
                                 "open-with-custom" });   // 打开方式

        // empty trash
        ret.insert("display-as",
                   QStringList { "display-as-icon",
                                 "display-as-list" });   // 显示方式

        ret.insert("sort-by",
                   QStringList { "sort-by-name",
                                 "sort-by-path",
                                 "sort-by-source-path",
                                 "sort-by-lastRead",
                                 "sort-by-time-modified",
                                 "sort-by-time-deleted",
                                 "sort-by-size",
                                 "sort-by-type" });   // 排序方式

        ret.insert("icon-size",
                   QStringList { "tiny",
                                 "small",
                                 "medium",
                                 "large",
                                 "super-large" });   // icon size

        ret.insert("stage-file-to-burning",
                   stageToRule());   // 添加至光盘刻录

        QStringList sendToList;
        sendToList << "send-to-bluetooth"
                   << ActionID::kSeparatorLine
                   << "send-to-desktop"
                   << "create-system-link"
                   << ActionID::kSeparatorLine
                   << sendToRule();

        ret.insert("send-to",
                   sendToList);   // 发送到
    });

    return ret;
}

BaseSortMenuScene::BaseSortMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new BaseSortMenuScenePrivate(this))
{
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
    d->sortPrimaryMenu(parent);
    d->sortSecondaryMenu(parent);
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
