// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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

FileInfoPointer DesktopFileCreator::createFileInfo(const QUrl &url, dfmbase::Global::CreateFileInfoType cache)
{
    QString errString;
    auto itemInfo = InfoFactory::create<FileInfo>(url, cache, &errString);
    if (Q_UNLIKELY(!itemInfo)) {
        fmInfo() << "create FileInfo error: " << errString << url;
        return nullptr;
    }

    return itemInfo;
}

DesktopFileCreator::DesktopFileCreator()
{
}
