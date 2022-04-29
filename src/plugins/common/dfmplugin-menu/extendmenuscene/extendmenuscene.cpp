/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#include "private/extendmenuscene_p.h"
#include "extendmenuscene/extendmenu/dcustomactionbuilder.h"
#include "extendmenuscene/extendmenu/dcustomactionparser.h"

#include <services/common/menu/menu_defines.h>
#include <dfm-base/base/schemefactory.h>

#include <QMenu>
#include <QDebug>

DPMENU_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE

AbstractMenuScene *ExtendMenuCreator::create()
{
    return new ExtendMenuScene();
}

ExtendMenuScenePrivate::ExtendMenuScenePrivate(ExtendMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
}

ExtendMenuScene::ExtendMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new ExtendMenuScenePrivate(this))
{
}

QString ExtendMenuScene::name() const
{
    return ExtendMenuCreator::name();
}

bool ExtendMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
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
        d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(d->focusFile, true, &errString);
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

    if (d->extendActions.contains(action))
        return const_cast<ExtendMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool ExtendMenuScene::create(QMenu *parent)
{
    d->extendActions.clear();
    d->cacheLocateActions.clear();
    d->cacheActionsSeparator.clear();

    const QList<DCustomActionEntry> &rootEntry = CustomParserIns->getActionFiles(d->onDesktop);

    qDebug() << "extendCustomMenu " << !d->isEmptyArea << d->currentDir << d->focusFile << "files" << d->selectFiles.size() << "entrys" << rootEntry.size();

    if (parent == nullptr || rootEntry.isEmpty())
        return false;

    DCustomActionBuilder builder;
    //呼出菜单的文件夹
    builder.setActiveDir(d->currentDir);

    //获取文件列表的组合
    DCustomActionDefines::ComboType fileCombo = DCustomActionDefines::BlankSpace;
    if (!d->isEmptyArea) {
        fileCombo = builder.checkFileCombo(d->selectFiles);
        if (fileCombo == DCustomActionDefines::BlankSpace)
            return false;

        //右键单击作用的文件
        builder.setFocusFile(d->focusFile);
    }

    //获取支持的菜单项
    auto usedEntrys = builder.matchFileCombo(rootEntry, fileCombo);

    //匹配类型支持
    usedEntrys = builder.matchActions(d->selectFiles, usedEntrys);
    qDebug() << "selected combo" << fileCombo << "entry count" << usedEntrys.size();

    if (usedEntrys.isEmpty())
        return false;

    //添加菜单响应所需的数据
    {
        QVariant var;
        var.setValue(d->currentDir);
        parent->setProperty(DCustomActionDefines::kCustomActionDataDir, var);

        var.setValue(d->focusFile);
        parent->setProperty(DCustomActionDefines::kCustomActionDataFoucsFile, var);

        var.setValue(d->selectFiles);
        parent->setProperty(DCustomActionDefines::kCustomActionDataSelectedFiles, var);
    }

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
        if (actionData.separator() != DCustomActionDefines::None)
            d->cacheActionsSeparator.insert(action, actionData.separator());

        //根据组合类型获取插入位置
        auto pos = actionData.position(fileCombo);

        //位置是否有效
        if (pos > 0) {
            auto temp = d->cacheLocateActions.find(pos);
            if (temp == d->cacheLocateActions.end()) {
                d->cacheLocateActions.insert(pos, { action });
            } else {   //位置冲突，往后放
                temp->append(action);
            }
        }

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

    bool hasPos;
    for (auto action : d->extendActions) {

        hasPos = false;
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
    DCustomActionDefines::sortFunc(d->cacheLocateActions, systemActions, [parent](const QList<QAction *> &acs) { parent->addActions(acs); }, [](QAction *ac) -> bool { return ac && !ac->isSeparator(); });

    Q_ASSERT(systemActions.isEmpty());

    //插入分隔线
    for (auto it = d->cacheActionsSeparator.begin(); it != d->cacheActionsSeparator.end(); ++it) {
        //上分割线
        if (it.value() & DCustomActionDefines::Top) {
            parent->insertSeparator(it.key());
        }

        //下分割线
        if ((it.value() & DCustomActionDefines::Bottom)) {
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
    if (!d->extendActions.contains(action))
        return false;

    QMenu *menu = static_cast<QMenu *>(action->parent());
    if (!menu || !action)
        return false;

    if (action->property(DCustomActionDefines::kCustomActionFlag).isValid()) {
        QString cmd = action->property(DCustomActionDefines::kCustomActionCommand).toString();
        DCustomActionDefines::ActionArg argFlag = static_cast<DCustomActionDefines::ActionArg>(action->property(DCustomActionDefines::kCustomActionCommandArgFlag).toInt());
        QUrl dir = menu->property(DCustomActionDefines::kCustomActionDataDir).value<QUrl>();
        QUrl foucs = menu->property(DCustomActionDefines::kCustomActionDataFoucsFile).value<QUrl>();
        QList<QUrl> selected = menu->property(DCustomActionDefines::kCustomActionDataSelectedFiles).value<QList<QUrl>>();

        qDebug() << "argflag" << argFlag << "dir" << dir << "foucs" << foucs << "selected" << selected;
        qInfo() << "extend" << action->text() << cmd;

        QPair<QString, QStringList> runable = DCustomActionBuilder::makeCommand(cmd, argFlag, dir, foucs, selected);
        qInfo() << "exec:" << runable.first << runable.second;

        // todo(wangcl)
        //        if (!runable.first.isEmpty())
        //            FileUtils::runCommand(runable.first, runable.second);
    }

    return AbstractMenuScene::triggered(action);
}
