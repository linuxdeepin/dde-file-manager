// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/extendmenuscene_p.h"

#include "extendmenuscene/extendmenu/dcustomactionbuilder.h"
#include "utils/menuhelper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-framework/dpf.h>

#include <QMenu>
#include <QDebug>

using namespace dfmplugin_menu;
DFMBASE_USE_NAMESPACE

static constexpr char kActionPosInMenu[] { "act_pos" };
static constexpr char kActionGroupId[] { "act_group_id" };

AbstractMenuScene *ExtendMenuCreator::create()
{
    std::call_once(loadFlag, [this]() {
        customParser = new DCustomActionParser(this);
        customParser->refresh();
        fmInfo() << "custom menus *.conf loaded.";
    });

    return new ExtendMenuScene(customParser);
}

ExtendMenuScenePrivate::ExtendMenuScenePrivate(ExtendMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
}

QList<QAction *> ExtendMenuScenePrivate::childActions(QAction *action)
{
    QList<QAction *> actions;

    if (action->menu()) {
        auto tempChildActions = action->menu()->actions();
        for (auto childAction : tempChildActions) {
            actions << childAction;
            actions << childActions(childAction);
        }
    }

    return actions;
}

int ExtendMenuScenePrivate::mayComboPostion(const DCustomActionData &acdata, DCustomActionDefines::ComboType combo)
{
    int pos = acdata.position(combo);
    // if kMultiDirs or kMultiFiles is not set pos, try to use kFileAndDir.
    if (combo == DCustomActionDefines::kMultiDirs
        || combo == DCustomActionDefines::kMultiFiles) {
        if (pos == acdata.position()) {
            pos = acdata.position(DCustomActionDefines::kFileAndDir);
        }
    }

    return pos;
}

void ExtendMenuScenePrivate::getSubMenus(QMenu *currMenu, const QString &parentMenuName, QMap<QString, QMenu *> &subMenus)
{
    Q_ASSERT(currMenu);

    auto actions = currMenu->actions();
    for (auto act : actions) {
        QString actID = act->property("actionID").toString();
        const auto &subMenu = act->menu();

        if (!actID.isEmpty() && subMenu) {
            if (!parentMenuName.isEmpty())
                actID.prepend(parentMenuName + "/");

            subMenus.insert(actID, subMenu);
            getSubMenus(subMenu, actID, subMenus);
        }
    }
}

bool ExtendMenuScenePrivate::insertIntoExistedSubActions(QAction *action, QMap<QString, QList<QAction *>> &extSubActMap)
{
    QString parentPath = action->property(DCustomActionDefines::kConfParentMenuPath).toString();

    bool foundParent = !parentPath.isEmpty() && extSubActMap.contains(parentPath);
    if (!foundParent)
        return false;

    auto subActions = extSubActMap.value(parentPath);
    bool hasPos = false;
    int pos = action->property(kActionPosInMenu).toInt(&hasPos);
    if (!hasPos)
        pos = -1;

    int actPos = -1;
    if (pos < 0 || subActions.isEmpty()) {
        actPos = subActions.count();
        subActions.append(action);
    } else {
        auto iter = std::find_if(subActions.begin(), subActions.end(), [pos](const QAction *act) {
            bool ok = false;
            int p = act->property(kActionPosInMenu).toInt(&ok);
            return !ok || p > pos;
        });

        if (iter == subActions.end()) {
            actPos = subActions.count();
            subActions.append(action);
        } else {
            subActions.insert(iter, action);
            actPos = subActions.indexOf(action);
        }
    }

    auto separatPos = cacheActionsSeparator.value(action, DCustomActionDefines::kNone);
    if (separatPos & DCustomActionDefines::kTop) {
        QAction *separator = new QAction(action->parent());
        separator->setSeparator(true);
        subActions.insert(actPos, separator);
        actPos++;   // separator inserted before action, the pos grows.
    }
    if (separatPos & DCustomActionDefines::kBottom) {
        QAction *separator = new QAction(action->parent());
        separator->setSeparator(true);
        subActions.insert(actPos + 1, separator);
    }

    cacheActionsSeparator.remove(action);
    extSubActMap.insert(parentPath, subActions);

    return true;
}

void ExtendMenuScenePrivate::mergeSubActions(const QMap<QString, QList<QAction *>> &extSubActMap, const QMap<QString, QMenu *> &subMenus)
{
    for (const auto &key : subMenus.keys()) {
        if (extSubActMap.value(key).isEmpty())
            continue;

        auto menu = subMenus.value(key);
        auto subActions = menu->actions();
        auto extSubActs = extSubActMap.value(key);

        // merge
        extSubActs.append(subActions);
        menu->addActions(extSubActs);
        auto holdingAct = menu->menuAction();
        if (holdingAct)
            holdingAct->setVisible(true);
    }
}

ExtendMenuScene::ExtendMenuScene(DCustomActionParser *parser, QObject *parent)
    : AbstractMenuScene(parent), d(new ExtendMenuScenePrivate(this))
{
    Q_ASSERT(parser);
    d->customParser = parser;
}

QString ExtendMenuScene::name() const
{
    return ExtendMenuCreator::name();
}

bool ExtendMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    UniversalUtils::urlTransformToLocal(d->currentDir, &d->transformedCurrentDir);
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    UniversalUtils::urlsTransformToLocal(d->selectFiles, &d->transformedSelectFiles);
    Q_ASSERT(d->selectFiles.size() == d->transformedSelectFiles.size());
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    if (!d->transformedSelectFiles.isEmpty())
        d->transformedFocusFile = d->transformedSelectFiles.first();

    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->indexFlags = params.value(MenuParamKey::kIndexFlags).value<Qt::ItemFlags>();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    if (!d->initializeParamsIsValid()) {
        fmWarning() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
        return false;
    }

    if (!d->isEmptyArea) {
        QString errString;
        d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(d->focusFile, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
        if (d->focusFileInfo.isNull()) {
            fmDebug() << errString;
            return false;
        }
    }

    return AbstractMenuScene::initialize(params);
}

AbstractMenuScene *ExtendMenuScene::scene(QAction *action) const
{
    if (!action)
        return nullptr;

    if (d->extendActions.contains(action) || d->extendChildActions.contains(action))
        return const_cast<ExtendMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool ExtendMenuScene::create(QMenu *parent)
{
    d->extendActions.clear();
    d->extendChildActions.clear();
    d->cacheLocateActions.clear();
    d->cacheActionsSeparator.clear();

    const QList<DCustomActionEntry> &rootEntry = d->customParser->getActionFiles(d->onDesktop);

    fmDebug() << "extendCustomMenu " << !d->isEmptyArea << d->currentDir << d->focusFile << "files" << d->selectFiles.size() << "entrys" << rootEntry.size();

    if (parent == nullptr || rootEntry.isEmpty())
        return AbstractMenuScene::create(parent);

    DCustomActionBuilder builder;
    //呼出菜单的文件夹
    builder.setActiveDir(d->currentDir);

    //获取文件列表的组合
    DCustomActionDefines::ComboType fileCombo = DCustomActionDefines::kBlankSpace;
    if (!d->isEmptyArea) {
#ifdef MENU_CHECK_FOCUSONLY
        fileCombo = builder.checkFileComboWithFocus(d->focusFile, d->selectFiles);
#else
        fileCombo = builder.checkFileCombo(d->selectFiles);
#endif
        if (fileCombo == DCustomActionDefines::kBlankSpace)
            return false;

        //右键单击作用的文件
        builder.setFocusFile(d->focusFile);
    }

    //获取支持的菜单项
    auto usedEntrys = builder.matchFileCombo(rootEntry, fileCombo);

    //匹配类型支持
#ifdef MENU_CHECK_FOCUSONLY
    usedEntrys = builder.matchActions({ d->focusFile }, usedEntrys);
#else
    usedEntrys = builder.matchActions(d->selectFiles, usedEntrys);
#endif
    fmDebug() << "selected combo" << fileCombo << "entry count" << usedEntrys.size();

    if (usedEntrys.isEmpty())
        return AbstractMenuScene::create(parent);

    //开启tooltips
    parent->setToolTipsVisible(true);

    //根据配置信息创建菜单项
    for (auto it = usedEntrys.begin(); it != usedEntrys.end(); ++it) {
        const DCustomActionData &actionData = it->data();
        auto *action = builder.buildAciton(actionData, parent);
        if (action == nullptr)
            continue;
        //自动释放
        action->setParent(parent);

        auto actions = d->childActions(action);
        d->extendChildActions.append(actions);

        bool isExist = false;
        for (auto existAction : d->extendActions) {
            //合并名字相同的菜单且都有子菜单的一级菜单
            if (action->text() == existAction->text()
                && existAction->menu()
                && action->menu()) {
                isExist = true;
                for (QAction *subaction : action->menu()->actions()) {
                    existAction->menu()->addAction(subaction);
                }
            }
        }
        if (isExist)
            continue;

        //记录分隔线
        if (actionData.separator() != DCustomActionDefines::kNone)
            d->cacheActionsSeparator.insert(action, actionData.separator());

            //根据组合类型获取插入位置
#ifdef MENU_CHECK_FOCUSONLY
        auto pos = d->mayComboPostion(actionData, fileCombo);
#else
        auto pos = actionData.position(fileCombo);
#endif

        //位置是否有效
        if (pos > 0) {
            d->cacheLocateActions[pos].append(action);
            action->setProperty(kActionPosInMenu, pos);
        }

        d->extendActions.append(action);
    }

    return AbstractMenuScene::create(parent);
}

void ExtendMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;

    // get all ID-ed submenus from parent
    QMap<QString, QMenu *> existedMenus;
    d->getSubMenus(parent, "", existedMenus);

    auto systemActions = parent->actions();
    for (auto it = systemActions.begin(); it != systemActions.end(); ++it)
        parent->removeAction(*it);
    Q_ASSERT(parent->actions().isEmpty());

    QMap<QString, QList<QAction *>> extendSubActions;
    for (auto key : existedMenus.keys())
        extendSubActions.insert(key, {});

    for (auto action : d->extendActions) {
        bool handled = d->insertIntoExistedSubActions(action, extendSubActions);

        bool hasPos = false;
        for (auto pos : d->cacheLocateActions.keys()) {
            auto posActions = d->cacheLocateActions.value(pos);
            if (posActions.contains(action)) {
                hasPos = true;
                if (handled) {
                    posActions.removeAll(action);
                    d->cacheLocateActions.insert(pos, posActions);
                }
                break;
            }
        }

        // 该action已记录位置，从位置缓存列表进行添加
        if (hasPos || handled)
            continue;

        // 未配置位置的项，追加到已有菜单项之后
        systemActions << action;
    }

    // 将配置菜单合并到子菜单项的前面
    d->mergeSubActions(extendSubActions, existedMenus);

    //开始按顺序插入菜单
    DCustomActionDefines::sortFunc(
            d->cacheLocateActions, systemActions,
            [parent](const QList<QAction *> &acs) { parent->addActions(acs); },
            [](QAction *ac) -> bool { return ac && !ac->isSeparator(); });

    // 确保同组的菜单项相邻，不被其他菜单项分隔
    QMap<QString, QList<QAction *>> groups;
    for (auto act : parent->actions()) {
        QString groupId = act->property(kActionGroupId).toString();
        if (!groupId.isEmpty())
            groups[groupId].append(act);
    }
    for (const auto &groupActions : groups) {
        if (groupActions.size() < 2)
            continue;
        // 将组内第二个及之后的项移到第一个项之后
        QAction *prevAct = groupActions.first();
        for (int i = 1; i < groupActions.size(); ++i) {
            QAction *act = groupActions[i];
            parent->removeAction(act);
            QList<QAction *> acts = parent->actions();
            int idx = acts.indexOf(prevAct);
            if (idx + 1 < acts.size())
                parent->insertAction(acts[idx + 1], act);
            else
                parent->addAction(act);
            prevAct = act;
        }
    }

    Q_ASSERT(systemActions.isEmpty());

    //插入分隔线
    for (auto it = d->cacheActionsSeparator.begin(); it != d->cacheActionsSeparator.end(); ++it) {
        //上分割线
        if (it.value() & DCustomActionDefines::kTop) {
            parent->insertSeparator(it.key());
        }

        //下分割线
        if ((it.value() & DCustomActionDefines::kBottom)) {
            const QList<QAction *> &actionList = parent->actions();
            int nextIndex = actionList.indexOf(it.key()) + 1;

            //后一个action
            if (nextIndex < actionList.size()) {
                auto nextAction = parent->actions().value(nextIndex);

                //不是分割线则插入
                if (!nextAction->isSeparator()) {
                    parent->insertSeparator(nextAction);
                }
            }
        }
    }

    AbstractMenuScene::updateState(parent);
}

bool ExtendMenuScene::triggered(QAction *action)
{
    if (!d->extendActions.contains(action) && !d->extendChildActions.contains(action))
        return AbstractMenuScene::triggered(action);

    if (Q_UNLIKELY(!action))
        return false;

    if (action->property(DCustomActionDefines::kCustomActionFlag).isValid()) {
        QString cmd = action->property(DCustomActionDefines::kCustomActionCommand).toString();
        DCustomActionDefines::ActionArg argFlag = static_cast<DCustomActionDefines::ActionArg>(action->property(DCustomActionDefines::kCustomActionCommandArgFlag).toInt());

        fmDebug() << "argflag" << argFlag << "dir" << d->currentDir << "foucs" << d->focusFile << "selected" << d->selectFiles;
        fmInfo() << "extend" << action->text() << cmd;

        QPair<QString, QStringList> runable = DCustomActionBuilder::makeCommand(
                cmd, argFlag, d->transformedCurrentDir, d->transformedFocusFile, d->transformedSelectFiles);
        fmInfo() << "exec:" << runable.first << runable.second;

        if (!runable.first.isEmpty())
            return UniversalUtils::runCommand(runable.first, runable.second);
    }

    return AbstractMenuScene::triggered(action);
}
