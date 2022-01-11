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

#include "dfm_desktop_service_global.h"
#include "dfm-base/interfaces/abstractmenu.h"
#include "services/common/menu/menuservice.h"

#include "dfm-framework/framework.h"

#include <QVariant>

class QMenu;

DSB_D_BEGIN_NAMESPACE

namespace MenuScene {
extern const char *const kDesktopMenu;
}   // namespace MenuScene

class CanvasMenu : public dfmbase::AbstractMenu
{
    enum DesktopCustomAction {
        kDisplaySettings = 1,
        kCornerSettings,
        kWallpaperSettings,
        kFileManagerProperty,
        kAutoMerge,
        kAutoSort,

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
    virtual QMenu *build(QWidget *parent,
                         MenuMode mode,
                         const QUrl &rootUrl,
                         const QUrl &foucsUrl,
                         const QList<QUrl> &selected = {},
                         QVariant customData = QVariant());

private:
    void emptyAreaMenu(QMenu *menu, const QUrl &rootUrl);

    void normalMenu(QMenu *menu,
                    const QUrl &rootUrl,
                    const QUrl &foucsUrl,
                    const QList<QUrl> &selected = {});

    void acitonBusiness(QAction *act);

private:
    DSC_NAMESPACE::MenuService *extensionMenuServer { nullptr };
    QMap<int, int> customActionType;
    QWidget *parentWidget { nullptr };
    QVariant cusData;
};

DSB_D_END_NAMESPACE
#endif   // CANVASMENU_H
