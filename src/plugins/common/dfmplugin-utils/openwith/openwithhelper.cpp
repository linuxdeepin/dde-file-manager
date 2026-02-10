// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openwithhelper.h"
#include "openwith/openwithwidget.h"

#include <dfm-base/base/schemefactory.h>

#include <dfm-framework/dpf.h>

Q_DECLARE_METATYPE(bool *)

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_utils;
OpenWithHelper::OpenWithHelper(QObject *parent)
    : QObject(parent)
{
}

QWidget *OpenWithHelper::createOpenWithWidget(const QUrl &url)
{
    if (url.isValid()) {
        FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(url);
        if (fileInfo.isNull())
            return nullptr;
        if (fileInfo->isAttributes(OptInfoType::kIsDir))
            return nullptr;
        bool pluginDisabledResult { false };
        dpfHookSequence->run("dfmplugin_utils", "hook_OpenWith_DisabledOpenWithWidget", url, &pluginDisabledResult);
        if (pluginDisabledResult)
            return nullptr;
        OpenWithWidget *openWidget = new OpenWithWidget;
        openWidget->selectFileUrl(url);
        return openWidget;
    }
    return nullptr;
}
