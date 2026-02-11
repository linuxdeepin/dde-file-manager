// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTICALEVENTCALLER_H
#define OPTICALEVENTCALLER_H

#include "dfmplugin_optical_global.h"

#include <QUrl>
#include <QWidget>

namespace dfmplugin_optical {

class OpticalEventCaller
{
    OpticalEventCaller() = delete;

public:
    static void sendOpenBurnDlg(const QString &dev, bool isSupportedUDF, QWidget *parent);
    static void sendOpenDumpISODlg(const QString &devId);
};

}

#endif   // OPTICALEVENTCALLER_H
