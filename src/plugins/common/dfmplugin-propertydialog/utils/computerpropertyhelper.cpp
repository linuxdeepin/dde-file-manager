// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "computerpropertyhelper.h"
#include "views/computerpropertydialog.h"
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_propertydialog;
QString ComputerPropertyHelper::scheme()
{
    return Global::Scheme::kComputer;
}

QWidget *ComputerPropertyHelper::createComputerProperty(const QUrl &url)
{
    static ComputerPropertyDialog *widget = nullptr;
    QUrl tempUrl;
    tempUrl.setPath("/");
    tempUrl.setScheme(scheme());
    bool flg = UniversalUtils::urlEquals(tempUrl, url);
    if (flg || FileUtils::isComputerDesktopFile(url)) {
        if (!widget) {
            widget = new ComputerPropertyDialog;
            return widget;
        }
        return widget;
    }
    return nullptr;
}
