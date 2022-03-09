/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef OPTICALMENU_H
#define OPTICALMENU_H

#include "dfmplugin_optical_global.h"

#include "dfm-base/interfaces/abstractmenu.h"
#include "dfm-base/widgets/action/actiondatacontainer.h"
#include "dfm-base/dfm_actiontype_defines.h"

#include <QUrl>

DPOPTICAL_BEGIN_NAMESPACE

namespace OpticalScene {
extern const char *const kOpticalMenu;
}   // namespace OpticalScene

class OpticalMenu : public DFMBASE_NAMESPACE::AbstractMenu
{
public:
    explicit OpticalMenu(QObject *parent = nullptr);
    QMenu *build(QWidget *parent,
                 MenuMode mode,
                 const QUrl &rootUrl,
                 const QUrl &focusUrl,
                 const QList<QUrl> &selected = {},
                 QVariant customData = QVariant()) override;

private:
    void assemblesEmptyAreaActions(QMenu *menu);
    void assemblesNormalActions(QMenu *menu);
    void filterActions(QMenu *menu, const QVector<DFMBASE_NAMESPACE::ActionType> &typeList, bool reverse = false);

private:
    QUrl curFocusUrl;
    QList<QUrl> curSelectedUrls;
};

DPOPTICAL_END_NAMESPACE

#endif   // OPTICALMENU_H
