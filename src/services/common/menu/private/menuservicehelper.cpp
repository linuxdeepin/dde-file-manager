/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#include "menuservicehelper.h"
#include "menu/template/templateaction.h"
#include "services/common/menu/customization/customactionbuilder.h"
#include "services/common/menu/customization/customactionparser.h"

#include "dfm-base/dfm_actiontype_defines.h"

#include <QAction>

namespace MenuData {
static CustomActionParser *customMenuParser = nullptr;
}

DSC_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

void MenuServiceHelper::templateMenu(QMenu *menu)
{
    QList<ActionDataContainer> actDataLst = TemplateAction::instance().getTemplateFileList();
    if (actDataLst.isEmpty())
        return;
    // TODO(Lee) 效率问题

    QList<QAction *> actLst = menu->actions();
    for (QAction *temp : actLst) {
        if (temp->data().toInt() == ActionType::kActNewDocument) {
            QMenu *tempDocMenu = temp->menu();
            if (!tempDocMenu) {
                tempDocMenu = new QMenu(menu);
                temp->setMenu(tempDocMenu);
            }
            for (auto &tempAct : actDataLst) {
                // Separator
                if (tempAct.actionType() == ActionType::kActSeparator) {
                    tempDocMenu->addSeparator();
                    continue;
                }

                if (tempAct.name().isEmpty() || (-1 == tempAct.actionType()))
                    continue;

                QAction *act = new QAction(tempDocMenu);
                act->setData(tempAct.actionType());
                act->setText(tempAct.name());
                if (!tempAct.icon().isNull())
                    act->setIcon(tempAct.icon());
                tempDocMenu->addAction(act);
            }
            break;
        }
    }
}

void MenuServiceHelper::desktopFileMenu(QMenu *menu)
{
    // TODO(Lee):
    menu->addAction("oem action");
}

void MenuServiceHelper::extendCustomMenu(QMenu *menu,
                                         bool isNormal,
                                         const QUrl &dir,
                                         const QUrl &focusFile,
                                         const QList<QUrl> &selected,
                                         bool onDesktop)
{
    if (!MenuData::customMenuParser) {
        MenuData::customMenuParser = new CustomActionParser;
    }

    const QList<CustomActionEntry> &rootEntry = MenuData::customMenuParser->getActionFiles(onDesktop);
    if (menu == nullptr || rootEntry.isEmpty())
        return;

    CustomActionBuilder builder;
    //呼出菜单的文件夹
    builder.setActiveDir(dir);

    //获取文件列表的组合
    CustomActionDefines::ComboType fileCombo = CustomActionDefines::kBlankSpace;
    if (isNormal) {
        fileCombo = builder.checkFileCombo(selected);
        if (fileCombo == CustomActionDefines::kBlankSpace)
            return;

        //右键单击作用的文件
        builder.setFocusFile(focusFile);
    }

    //获取支持的菜单项
    auto usedEntrys = builder.matchFileCombo(rootEntry, fileCombo);

    //匹配类型支持
    usedEntrys = builder.matchActions(selected, usedEntrys);
    if (usedEntrys.isEmpty())
        return;

    //添加菜单响应所需的数据
    {
        QVariant var;
        var.setValue(dir);
        menu->setProperty(CustomActionDefines::kCustomActionDataDir, var);

        var.setValue(focusFile);
        menu->setProperty(CustomActionDefines::kCustomActionDataFoucsFile, var);

        var.setValue(selected);
        menu->setProperty(CustomActionDefines::kCustomActionDataSelectedFiles, var);
    }

    //开启tooltips
    menu->setToolTipsVisible(true);

    //移除所有菜单项
    auto systemActions = menu->actions();
    for (auto it = systemActions.begin(); it != systemActions.end(); ++it)
        menu->removeAction(*it);
    Q_ASSERT(menu->actions().isEmpty());

    QMap<int, QList<QAction *>> locate;
    QMap<QAction *, CustomActionDefines::Separator> actionsSeparator;
    //根据配置信息创建菜单项
    for (auto it = usedEntrys.begin(); it != usedEntrys.end(); ++it) {
        const CustomActionData &actionData = it->data();
        auto *action = builder.buildAciton(actionData, menu);
        if (action == nullptr)
            continue;

        //自动释放
        action->setParent(menu);

        //记录分隔线
        if (actionData.separator() != CustomActionDefines::kNone)
            actionsSeparator.insert(action, actionData.separator());

        //根据组合类型获取插入位置
        auto pos = actionData.position(fileCombo);

        //位置是否有效
        if (pos > 0) {
            auto temp = locate.find(pos);
            if (temp == locate.end()) {
                locate.insert(pos, { action });
            } else {   //位置冲突，往后放
                temp->append(action);
            }
        } else {   //没有配置位置，则直接添加
            systemActions.append(action);
        }
    }

    //开始按顺序插入菜单
    CustomActionDefines::sortFunc(locate, systemActions, [menu](const QList<QAction *> &acs) { menu->addActions(acs); }, [](QAction *ac) -> bool { return ac && !ac->isSeparator(); });

    Q_ASSERT(systemActions.isEmpty());

    //插入分隔线
    for (auto it = actionsSeparator.begin(); it != actionsSeparator.end(); ++it) {
        //上分割线
        if (it.value() & CustomActionDefines::kTop) {
            menu->insertSeparator(it.key());
        }

        //下分割线
        if ((it.value() & CustomActionDefines::kBottom)) {
            const QList<QAction *> &actionList = menu->actions();
            int nextIndex = actionList.indexOf(it.key()) + 1;

            //后一个action
            if (nextIndex < actionList.size()) {

                auto nextAction = menu->actions().value(nextIndex);

                //不是分割线则插入
                if (!nextAction->isSeparator()) {
                    menu->insertSeparator(nextAction);
                }
            }
        }
    }
}

void MenuServiceHelper::extensionPluginCustomMenu(QMenu *menu,
                                                  bool isNormal,
                                                  const QUrl &currentUrl,
                                                  const QUrl &focusFile,
                                                  const QList<QUrl> &selected)
{
    // TODO(Lee):
    if (extActionEnable()) {
        for (const ActionInfo &info : actionInfos()) {
            if (info.type != DFMBASE_NAMESPACE::ExtensionType::kSoAction)
                continue;

            if (!info.createCb) {
                qCritical() << "action create callback not set";
            }

            QString name = info.createCb(isNormal, currentUrl, focusFile, selected);
            if (name.isEmpty())
                continue;

            if (!info.clickedCb) {
                qCritical() << "action cliecked callback not set";
                abort();
            }
            QAction *action = menu->addAction(name);
            QObject::connect(action, &QAction::triggered, [&info, isNormal, currentUrl, focusFile, selected](){
                info.clickedCb(isNormal, currentUrl, focusFile, selected);
            });
        }
    }
}

void MenuServiceHelper::regAction(ActionInfo &info)
{
    extActionEnable() = true;
    actionInfos() << info;
}

bool &MenuServiceHelper::extActionEnable()
{
    static bool hasAction { false };
    return hasAction;
}

QList<ActionInfo> &MenuServiceHelper::actionInfos()
{
    static QList<ActionInfo> infos;
    return infos;
}

DSC_END_NAMESPACE
