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
#ifndef CANVASMENU_H
#define CANVASMENU_H

#include "ddplugin_canvas_global.h"

#include <services/common/menu/menuservice.h>

#include <interfaces/abstractmenu.h>
#include <interfaces/abstractfileinfo.h>
#include <widgets/action/actiondatacontainer.h>

#include <dfm-framework/framework.h>

#include <QVariant>

class QMenu;

namespace MenuScene {
extern const char *const kDesktopMenu;
}   // namespace MenuScene

DDP_CANVAS_BEGIN_NAMESPACE

class CanvasView;
class CanvasMenu : public DFMBASE_NAMESPACE::AbstractMenu
{
    enum DesktopCustomAction {
        kDisplaySettings = DFMBASE_NAMESPACE::ActionType::kActMaxCustom,
        kCornerSettings,
        kWallpaperSettings,
        kFileManagerProperty,
        kAutoMerge,
        kAutoArrange,
        kIconSize,
        kIconSize0 = kIconSize,
        kIconSize1 = kIconSize + 1,
        kIconSize2 = kIconSize + 2,
        kIconSize3 = kIconSize + 3,
        kIconSize4 = kIconSize + 4,
    };
    Q_ENUM(DesktopCustomAction)

public:
    CanvasMenu();
    ~CanvasMenu();
    virtual QMenu *build(QWidget *parent,
                         MenuMode mode,
                         const QUrl &rootUrl,
                         const QUrl &focusUrl,
                         const QList<QUrl> &selected = {},
                         QVariant customData = QVariant());

protected:
    void emptyAreaMenu(QMenu *menu, const QUrl &rootUrl);

    void normalMenu(QMenu *menu,
                    const QUrl &rootUrl,
                    const QUrl &focusUrl,
                    const QList<QUrl> &selected = {});

    void actionBusiness(QAction *act);
    void registDesktopCustomActions();
    void registDesktopCustomSubActions();
    void columnRolesAssociateActionType();
    void creatMenuByDataLst(QMenu *menu, const QVector<DFMBASE_NAMESPACE::ActionDataContainer> &lst);
    void getActionDataByTypes(QVector<DFMBASE_NAMESPACE::ActionDataContainer> &lst,
                              const QVector<DFMBASE_NAMESPACE::ActionType> &types,
                              const QSet<DFMBASE_NAMESPACE::ActionType> &unUsedTypes);
    void setActionSpecialHandling(QMenu *menu);
    bool isRefreshOn() const;

    DFMBASE_NAMESPACE::ActionDataContainer getSendToMenu(bool hasFolder);

private:
    DSC_NAMESPACE::MenuService *extensionMenuServer { nullptr };
    QMap<DesktopCustomAction, DFMBASE_NAMESPACE::ActionDataContainer> customAction;
    QMap<int, DesktopCustomAction> customActionType;
    QMap<int, QUrl> sendToRemovabalDiskActs;
    CanvasView *view { nullptr };
    QVariant cusData;
    QMap<DFMBASE_NAMESPACE::AbstractFileInfo::SortKey, DFMBASE_NAMESPACE::ActionType> userColumnRoles;
    bool actionTypesInitialized { false };
    DFMBASE_NAMESPACE::ActionDataContainer *sortByActionData { nullptr };
};

DDP_CANVAS_END_NAMESPACE
#endif   // CANVASMENU_H
