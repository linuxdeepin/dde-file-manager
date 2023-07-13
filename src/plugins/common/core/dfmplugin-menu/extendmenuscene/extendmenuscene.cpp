// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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

AbstractMenuScene *ExtendMenuCreator::create()
{
    std::call_once(loadFlag, [this]() {
        customParser = new DCustomActionParser(this);
        customParser->refresh();
        qInfo() << "custom menus *.conf loaded.";
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
        qWarning() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
        return false;
    }

    if (!d->isEmptyArea) {
        QString errString;
        d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(d->focusFile, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
        if (d->focusFileInfo.isNull()) {
            qDebug() << errString;
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

    qDebug() << "extendCustomMenu " << !d->isEmptyArea << d->currentDir << d->focusFile << "files" << d->selectFiles.size() << "entrys" << rootEntry.size();

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
    usedEntrys = builder.matchActions({d->focusFile}, usedEntrys);
#else
    usedEntrys = builder.matchActions(d->selectFiles, usedEntrys);
#endif
    qDebug() << "selected combo" << fileCombo << "entry count" << usedEntrys.size();

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
            auto temp = d->cacheLocateActions.find(pos);
            if (temp == d->cacheLocateActions.end()) {
                d->cacheLocateActions.insert(pos, { action });
            } else {   //位置冲突，往后放
                temp->append(action);
            }
        }

        auto actions = d->childActions(action);
        d->extendChildActions.append(actions);

        d->extendActions.append(action);
    }

    return AbstractMenuScene::create(parent);
}

void ExtendMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;

    auto systemActions = parent->actions();
    for (auto it = systemActions.begin(); it != systemActions.end(); ++it)
        parent->removeAction(*it);
    Q_ASSERT(parent->actions().isEmpty());

    for (auto action : d->extendActions) {

        bool hasPos = false;
        for (auto pos : d->cacheLocateActions.keys()) {
            auto posActions = d->cacheLocateActions.value(pos);
            if (posActions.contains(action)) {
                hasPos = true;
                break;
            }
        }

        // 该action已记录位置，从位置缓存列表进行添加
        if (hasPos)
            continue;

        // 未配置位置的项，追加到已有菜单项之后
        systemActions << action;
    }

    //开始按顺序插入菜单
    DCustomActionDefines::sortFunc(
            d->cacheLocateActions, systemActions,
            [parent](const QList<QAction *> &acs) { parent->addActions(acs); },
            [](QAction *ac) -> bool { return ac && !ac->isSeparator(); });

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

        qDebug() << "argflag" << argFlag << "dir" << d->currentDir << "foucs" << d->focusFile << "selected" << d->selectFiles;
        qInfo() << "extend" << action->text() << cmd;

        QPair<QString, QStringList> runable = DCustomActionBuilder::makeCommand(
                cmd, argFlag, d->transformedCurrentDir, d->transformedFocusFile, d->transformedSelectFiles);
        qInfo() << "exec:" << runable.first << runable.second;

        if (!runable.first.isEmpty())
            return UniversalUtils::runCommand(runable.first, runable.second);
    }

    return AbstractMenuScene::triggered(action);
}
