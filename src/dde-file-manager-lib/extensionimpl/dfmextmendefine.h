/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             huangyu<zhangyub@uniontech.com>
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
#ifndef DFMEXTMENDEFINE_H
#define DFMEXTMENDEFINE_H

#include <QtCore>

#define ID_EXTMENU_P "ID_EXTQMENU_PRIVATE"
#define ID_EXTACTION_P "ID_EXTQACTION_PRIVATE"

#define DFMExt_Get_MenuPrivate(qmenu) \
        qmenu->property(ID_EXTMENU_P).value<DFMExtMenuImplPrivate *>()

#define DFMExt_Install_MenuPrivate(qmenu, extmenu_p) \
        Q_ASSERT(DFMExt_Get_MenuPrivate(qmenu) == nullptr); \
        qmenu->setProperty(ID_EXTMENU_P, QVariant::fromValue(extmenu_p))

#define DFMExt_Get_ActionPrivate(qaction) \
        qaction->property(ID_EXTACTION_P).value<DFMExtActionImplPrivate *>()

#define DFMExt_Install_ActionPrivate(qaction, extaction_p) \
        Q_ASSERT(DFMExt_Get_ActionPrivate(qaction) == nullptr); \
        qaction->setProperty(ID_EXTACTION_P, QVariant::fromValue(extaction_p))

#endif // DFMEXTMENDEFINE_H
