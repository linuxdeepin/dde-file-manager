// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MENUHELPER_H
#define MENUHELPER_H

#include "dfmplugin_menu_global.h"

#include <QUrl>

namespace dfmplugin_menu {
namespace Helper {

bool isHiddenExtMenuByDConfig(const QUrl &dirUrl);

}   //  namespace Helper
}   //  namespace dfmplugin_menu
#endif   // MENUHELPER_H
