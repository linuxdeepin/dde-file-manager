// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BURNEVENTCALLER_H
#define BURNEVENTCALLER_H

#include "dfmplugin_burn_global.h"

#include <QUrl>

namespace dfmplugin_burn {

class BurnEventCaller
{
    BurnEventCaller() = delete;

public:
    static void sendPasteFiles(const QList<QUrl> &urls, const QUrl &dest, bool isCopy);
};

}

#endif   // BURNEVENTCALLER_H
