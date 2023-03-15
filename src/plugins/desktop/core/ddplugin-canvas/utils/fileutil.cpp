// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileutil.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/desktopfileinfo.h>

#include <QReadWriteLock>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

class DesktopFileCreatorGlogal : public DesktopFileCreator
{
};
Q_GLOBAL_STATIC(DesktopFileCreatorGlogal, desktopFileCreatorGlogal)

DesktopFileCreator *DesktopFileCreator::instance()
{
    return desktopFileCreatorGlogal;
}

DFMLocalFileInfoPointer DesktopFileCreator::createFileInfo(const QUrl &url, bool cache)
{
    QString errString;
    auto itemInfo = InfoFactory::create<LocalFileInfo>(url, cache, &errString);
    if (Q_UNLIKELY(!itemInfo)) {
        qInfo() << "create LocalFileInfo error: " << errString << url;
        return nullptr;
    }

    return itemInfo;
}

DesktopFileCreator::DesktopFileCreator()
{
}
