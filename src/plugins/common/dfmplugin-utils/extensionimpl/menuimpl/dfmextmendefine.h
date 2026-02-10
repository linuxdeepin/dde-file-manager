// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTMENDEFINE_H
#define DFMEXTMENDEFINE_H

#include <QtCore>

#define ID_EXTMENU_P "ID_EXTQMENU_PRIVATE"
#define ID_EXTACTION_P "ID_EXTQACTION_PRIVATE"

#define DFMExt_Get_MenuPrivate(qmenu) \
    qmenu->property(ID_EXTMENU_P).value<DFMExtMenuImplPrivate *>()

#define DFMExt_Install_MenuPrivate(qmenu, extmenu_p)    \
    Q_ASSERT(DFMExt_Get_MenuPrivate(qmenu) == nullptr); \
    qmenu->setProperty(ID_EXTMENU_P, QVariant::fromValue(extmenu_p))

#define DFMExt_Get_ActionPrivate(qaction) \
    qaction->property(ID_EXTACTION_P).value<DFMExtActionImplPrivate *>()

#define DFMExt_Install_ActionPrivate(qaction, extaction_p)  \
    Q_ASSERT(DFMExt_Get_ActionPrivate(qaction) == nullptr); \
    qaction->setProperty(ID_EXTACTION_P, QVariant::fromValue(extaction_p))

#endif   // DFMEXTMENDEFINE_H
