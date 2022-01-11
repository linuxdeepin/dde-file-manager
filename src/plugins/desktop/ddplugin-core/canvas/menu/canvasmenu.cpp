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
#include "canvasmenu.h"
#include "canvas/view/canvasview.h"
#include "canvas/view/operator/fileoperaterproxy.h"
#include "canvas/canvasmanager.h"

#include "dfm-base/dfm_actiontype_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/local/localfileinfo.h"
#include "dfm-base/widgets/action/actiondatacontainer.h"
#include "dfm-base/utils/systempathutil.h"
#include "dfm-base/utils/actiontypemanager.h"
#include "dfm-base/file/fileAction/desktopfileactions.h"

#include <QMenu>

DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE
DSB_D_BEGIN_NAMESPACE

namespace MenuScene {
extern const char *const kDesktopMenu = "desktop-menu";
}   // namespace MenuScene

CanvasMenu::CanvasMenu()
{
    // 获取扩展菜单服务
    auto &ctx = dpfInstance.serviceContext();
    extensionMenuServer = ctx.service<MenuService>(MenuService::name());
}

QMenu *CanvasMenu::build(QWidget *parent,
                         AbstractMenu::MenuMode mode,
                         const QUrl &rootUrl,
                         const QUrl &foucsUrl,
                         const QList<QUrl> &selected,
                         QVariant customData)
{
    cusData = std::move(customData);
    parentWidget = parent;
    QMenu *menu = new QMenu(parent);
    if (AbstractMenu::MenuMode::kEmpty == mode) {

        Q_UNUSED(foucsUrl)
        Q_UNUSED(selected)
        emptyAreaMenu(menu, rootUrl);
    }

    if (AbstractMenu::MenuMode::kNormal == mode) {
        normalMenu(menu, rootUrl, foucsUrl, selected);
    }

    // Action业务
    connect(menu, &QMenu::triggered, this, [this](QAction *action) {
        qDebug() << "desktop menu trigered!";
        this->acitonBusiness(action);
    },
            Qt::QueuedConnection);
    return menu;
}

void CanvasMenu::emptyAreaMenu(QMenu *menu, const QUrl &rootUrl)
{
    QString errString;
    auto tempInfo = dfmbase::InfoFactory::create<AbstractFileInfo>(rootUrl, true, &errString);
    if (!tempInfo) {
        qInfo() << "create LocalFileInfo error: " << errString;
        return;
    }
    QVector<ActionType> tempLst;

    // 获取对应Scheme对应的菜单列表
    auto baseDec = QSharedPointer<AbstractFileInfo>(new AbstractFileActions(tempInfo));
    auto tempActionInfo = new DesktopFileActions(baseDec);

    // 使用tempActionInfo通过tempFileInfo获取对应的action信息
    tempLst = tempActionInfo->menuActionList(AbtMenuType::kSpaceArea);

    // 根据actiontype获取到action信息
    auto tempActTextLst = ActionTypeManager::instance().actionNameListByTypes(tempLst);

    // 增加桌面场景自定义的菜单
    // todo: registerActionType 返回键值对
    auto customAct = ActionTypeManager::instance().registerActionType(tr("Display Settings"));
    customActionType.insert(customAct.first, DesktopCustomAction::kDisplaySettings);
    tempActTextLst << customAct.second;

    customAct = ActionTypeManager::instance().registerActionType(tr("Wallpaper and Screensaver"));
    customActionType.insert(customAct.first, DesktopCustomAction::kWallpaperSettings);
    tempActTextLst << customAct.second;

    customAct = ActionTypeManager::instance().registerActionType(tr("Icon size"));
    customActionType.insert(customAct.first, DesktopCustomAction::kIconSize);
    tempActTextLst << customAct.second;

    if (tempActTextLst.isEmpty())
        return;

    // add action to menu
    for (auto &tempAct : tempActTextLst) {

        // Separator
        if (tempAct.actionType() == ActionType::kActSeparator) {
            menu->addSeparator();
            continue;
        }

        if (tempAct.name().isEmpty() || (-1 == tempAct.actionType()))
            continue;

        QAction *act = new QAction(menu);
        act->setData(tempAct.actionType());
        act->setText(tempAct.name());
        if (!tempAct.icon().isNull())
            act->setIcon(tempAct.icon());
        menu->addAction(act);
    }
}

void CanvasMenu::normalMenu(QMenu *menu,
                            const QUrl &rootUrl,
                            const QUrl &foucsUrl,
                            const QList<QUrl> &selected)
{
    Q_UNUSED(rootUrl)

    QString errString;
    auto tempInfo = dfmbase::InfoFactory::create<AbstractFileInfo>(foucsUrl, true, &errString);
    if (!tempInfo) {
        qInfo() << "create LocalFileInfo error: " << errString;
        return;
    }

    QVector<ActionType> tempLst;

    // 获取对应Scheme对应的菜单列表
    auto baseDec = QSharedPointer<AbstractFileInfo>(new AbstractFileActions(tempInfo));
    auto tempActionInfo = new DesktopFileActions(baseDec);

    // 使用tempActionInfo通过tempFileInfo获取对应的action信息
    if (selected.size() == 1) {
        tempLst = tempActionInfo->menuActionList(AbtMenuType::kSingleFile);
    } else {
        bool isSystemPathIncluded = false;
        for (auto &temp : selected) {
            if (SystemPathUtil::instance()->isSystemPath(temp.url())) {
                isSystemPathIncluded = true;
                break;
            }
        }
        if (isSystemPathIncluded) {
            tempLst = tempActionInfo->menuActionList(AbtMenuType::kMultiFilesSystemPathIncluded);
        } else {
            tempLst = tempActionInfo->menuActionList(AbtMenuType::kMultiFiles);
        }
    }

    // 根据actiontype获取到action信息
    auto tempActTextLst = ActionTypeManager::instance().actionNameListByTypes(tempLst);
    if (tempActTextLst.isEmpty())
        return;

    // add action to menu
    for (auto &tempAct : tempActTextLst) {

        // Separator
        if (tempAct.actionType() == ActionType::kActSeparator) {
            menu->addSeparator();
            continue;
        }

        if (tempAct.name().isEmpty() || (-1 == tempAct.actionType()))
            continue;

        QAction *act = new QAction(menu);
        act->setData(tempAct.actionType());
        act->setText(tempAct.name());
        if (!tempAct.icon().isNull())
            act->setIcon(tempAct.icon());
        menu->addAction(act);
    }

    // TODO(Lee)：多文件筛选、多选中包含 计算机 回收站 主目录时不显示扩展菜单
}

void CanvasMenu::acitonBusiness(QAction *act)
{
    auto actType = act->data().toInt();
    CanvasView *view = qobject_cast<CanvasView *>(parentWidget);

    {
        // 自定义业务
        int customType = customActionType.value(actType);
        switch (customType) {
        case DesktopCustomAction::kIconSize: {
            qDebug() << "desktop menu -> DisplaySettings!";
            return;
        }
        case DesktopCustomAction::kDisplaySettings: {
            qDebug() << "desktop menu -> Icon size!";
            return;
        }
        case DesktopCustomAction::kWallpaperSettings: {
            qDebug() << "desktop menu -> Wallpaper and Screensaver!";
            CanvasIns->onWallperSetting(view);
            return;
        }
        default:
            break;
        }
    }

    {
        // todo(lee):
        // 菜单事件(一些非事件的自定义业务处理直接在之前返回)

        qDebug() << "eventDispatcher -> action: " << act->text();
        switch (actType) {
        case kActNewFolder: {
            FileOperaterProxyIns->touchFolder(view, cusData.toPoint());
            return;
        }
        case kActDelete: {
            FileOperaterProxyIns->moveToTrash(view);
            return;
        }
        default:
            break;
        }
    }
}

DSB_D_END_NAMESPACE
