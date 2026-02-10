// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MENUHELPER_H
#define MENUHELPER_H

#include "dfmplugin_menu_global.h"

#include <QUrl>

namespace dfmplugin_menu {
namespace Helper {

bool isHiddenExtMenu(const QUrl &dirUrl);
bool isHiddenMenu(const QString &app);
bool isHiddenDesktopMenu();
bool canOpenSelectedItems(const QList<QUrl> &urlList);
}   //  namespace Helper
}   //  namespace dfmplugin_menu
#endif   // MENUHELPER_H
